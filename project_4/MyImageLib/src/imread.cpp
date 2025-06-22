#include "Mat.h"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <omp.h>
const int PARALLEL_THRESHOLD = 256 * 256;
#pragma pack(push, 1)
struct BMPFileHeader
{
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
};
struct BMPInfoHeader
{
    uint32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
};
#pragma pack(pop)

inline void process8bitLine(
    const unsigned char *rowPtr,
    unsigned char *dstLine,
    int width,
    int channels,
    const unsigned char *palette,
    bool isGray8,
    bool forceRGB)
{
    for (int x = 0; x < width; ++x)
    {
        unsigned char index = rowPtr[x];
        const unsigned char *color = palette + (index << 2);
        unsigned char *out = dstLine + x * channels;

        if (isGray8 && !forceRGB)
            *out = *color;
        else
        {
            out[0] = color[0];
            out[1] = color[1];
            out[2] = color[2];
        }
    }
}

inline void process1bitLine(
    const unsigned char *rowPtr,
    unsigned char *dstLine,
    int width,
    const unsigned char *palette)
{
    for (int x = 0; x < width; ++x)
    {
        const unsigned char *bytePtr = rowPtr + (x >> 3);
        int bitIdx = 7 - (x & 7);
        unsigned char bit = (*bytePtr >> bitIdx) & 1;
        const unsigned char *color = palette + (bit << 2);
        unsigned char *out = dstLine + x * 3;

        out[0] = color[0]; // R
        out[1] = color[1]; // G
        out[2] = color[2]; // B
    }
}

Mat imread(const std::string &filename)
{
#pragma omp parallel
    {
#pragma omp single
        std::cout << "[OpenMP] Running with " << omp_get_num_threads() << " threads\n";
    }

    std::ifstream file(filename, std::ios::binary);
    if (!file)
        throw std::runtime_error("Cannot open BMP file: " + filename);

    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;
    file.read(reinterpret_cast<char *>(&fileHeader), sizeof(fileHeader));
    file.read(reinterpret_cast<char *>(&infoHeader), sizeof(infoHeader));

    if (fileHeader.bfType != 0x4D42)
        throw std::runtime_error("Not a BMP file.");
    if (infoHeader.biCompression != 0)
        throw std::runtime_error("Compressed BMP not supported.");

    int width = infoHeader.biWidth;
    int height = std::abs(infoHeader.biHeight);
    int bitCount = infoHeader.biBitCount;
    bool topDown = infoHeader.biHeight < 0;

    unsigned char *palette = nullptr;
    int colorCount = 0;
    bool isGray8 = false;
    bool skipPaletteLookup = false;
    bool forceRGB = false;
    if (bitCount == 8 || bitCount == 1)
    {
        colorCount = infoHeader.biClrUsed ? infoHeader.biClrUsed : (1 << bitCount);
        palette = new unsigned char[colorCount * 4];
        file.read(reinterpret_cast<char *>(palette), colorCount * 4);

        if (bitCount == 8)
        {
            isGray8 = true;
            for (int i = 0; i < colorCount; ++i)
            {
                int base = i * 4;
                unsigned char R = palette[base + 2];
                unsigned char G = palette[base + 1];
                unsigned char B = palette[base + 0];
                if (R != G || G != B)
                {
                    isGray8 = false;
                    break;
                }
            }
            if (isGray8)
            {
                std::cout << "[INFO] Detected grayscale BMP image.\n";
                std::cout << "Do you want to convert it to RGB format? (y/n): ";
                char choice;
                std::cin >> choice;
                if (choice == 'y' || choice == 'Y')
                {
                    forceRGB = true;
                }
                else if (choice == 'n' || choice == 'N')
                {
                    forceRGB = false;
                    skipPaletteLookup = true;
                }
                else
                {
                    std::cerr << "[WARNING] Invalid choice. Defaulting to RGB.\n";
                    forceRGB = true;
                }
            }
        }
    }

    int channels = 0;
    if (bitCount == 32)
        channels = 4;
    else if (bitCount == 24)
        channels = 3;
    else if (bitCount == 8)
        channels = (isGray8 && !forceRGB) ? 1 : 3;
    else if (bitCount == 1)
        channels = 3;
    else
        throw std::runtime_error("Unsupported BMP bit count: " + std::to_string(bitCount));

    Mat image(width, height, channels);
    unsigned char *dst = image.getWritableData();
    using PixelProcessor = void (*)(const unsigned char *row, int x, int dstIdx, unsigned char *dst, const unsigned char *palette, bool isGray8, bool forceRGB);
    PixelProcessor processor = nullptr;

    int rowSizeBits = width * bitCount;
    int rowSizeBytes = ((rowSizeBits + 31) / 32) * 4;

    if (bitCount == 24 || bitCount == 32)
    {
        int rowSizeBytes = ((width * bitCount + 31) / 32) * 4; // 含 padding
        int rowRawBytes = width * channels;                    // 不含 padding

        size_t bufferSize = static_cast<size_t>(height) * rowSizeBytes;
        unsigned char *fullBuffer = new unsigned char[bufferSize];
        file.read(reinterpret_cast<char *>(fullBuffer), bufferSize);
        if (width * height >= PARALLEL_THRESHOLD)
        {
#pragma omp parallel for schedule(static)
            for (int y = 0; y < height; ++y)
            {
                const unsigned char *rowPtr = fullBuffer + y * rowSizeBytes;
                int dstY = topDown ? y : (height - 1 - y);
                unsigned char *dstLine = dst + dstY * rowRawBytes;
                std::memcpy(dstLine, rowPtr, rowRawBytes); // 跳过 padding
            }
        }
        else
        {
            for (int y = 0; y < height; ++y)
            {
                const unsigned char *rowPtr = fullBuffer + y * rowSizeBytes;
                int dstY = topDown ? y : (height - 1 - y);
                unsigned char *dstLine = dst + dstY * rowRawBytes;
                std::memcpy(dstLine, rowPtr, rowRawBytes);
            }
        }

        delete[] fullBuffer;
    }
    else if (bitCount == 8)
    {
        std::vector<unsigned char> buffer(height * rowSizeBytes);
        file.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
        if (width * height >= PARALLEL_THRESHOLD)
        {
#pragma omp parallel for schedule(static)
            for (int y = 0; y < height; ++y)
            {
                const unsigned char *rowPtr = buffer.data() + y * rowSizeBytes;
                int dstY = topDown ? y : (height - 1 - y);
                unsigned char *dstLine = dst + dstY * width * channels;
                process8bitLine(rowPtr, dstLine, width, channels, palette, isGray8, forceRGB);
            }
        }

        else
        {
            for (int y = 0; y < height; ++y)
            {
                const unsigned char *rowPtr = buffer.data() + y * rowSizeBytes;
                int dstY = topDown ? y : (height - 1 - y);
                unsigned char *dstLine = dst + dstY * width * channels;

                process8bitLine(rowPtr, dstLine, width, channels, palette, isGray8, forceRGB);
            }
        }
    }
    else if (bitCount == 1)
    {
        std::vector<unsigned char> buffer(height * rowSizeBytes);
        file.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
        if (width * height >= PARALLEL_THRESHOLD)
        {
#pragma omp for schedule(static)
            for (int y = 0; y < height; ++y)
            {
                const unsigned char *rowPtr = buffer.data() + y * rowSizeBytes;
                int dstY = topDown ? y : (height - 1 - y);
                unsigned char *dstLine = dst + dstY * width * channels;

                process1bitLine(rowPtr, dstLine, width, palette);
            }
        }
        else
        {
            for (int y = 0; y < height; ++y)
            {
                const unsigned char *rowPtr = buffer.data() + y * rowSizeBytes;
                int dstY = topDown ? y : (height - 1 - y);
                unsigned char *dstLine = dst + dstY * width * channels;

                process1bitLine(rowPtr, dstLine, width, palette);
            }
        }
    }

    delete[] palette; // 必须释放内存
    return image;
}