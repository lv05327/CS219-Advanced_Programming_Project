#include <Mat.h>
#include <fstream>
#include <cstring>
#include <omp.h>
#pragma pack(push, 1)
struct BMPFileHeader
{
    uint16_t bfType = 0x4D42; // 'BM'
    uint32_t bfSize = 0;
    uint16_t bfReserved1 = 0;
    uint16_t bfReserved2 = 0;
    uint32_t bfOffBits = 54;
};

struct BMPInfoHeader
{
    uint32_t biSize = 40;
    int32_t biWidth = 0;
    int32_t biHeight = 0;
    uint16_t biPlanes = 1;
    uint16_t biBitCount = 0;
    uint32_t biCompression = 0;
    uint32_t biSizeImage = 0;
    int32_t biXPelsPerMeter = 2835;
    int32_t biYPelsPerMeter = 2835;
    uint32_t biClrUsed = 0;
    uint32_t biClrImportant = 0;
};
#pragma pack(pop)

bool imwrite(const std::string &filename, const Mat &mat)
{
    int width = mat.getWidth();
    int height = mat.getHeight();
    int channels = mat.getChannels();
    const unsigned char *src = mat.getData();

    if (channels != 1 && channels != 3 && channels != 4)
        return false;

    int bitCount = channels * 8;
    int rowRawBytes = width * channels;
    int rowPaddedBytes = (rowRawBytes + 3) & ~3;
    int imageSize = rowPaddedBytes * height;

    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;

    infoHeader.biWidth = width;
    infoHeader.biHeight = height;
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = bitCount;
    infoHeader.biCompression = 0;
    infoHeader.biSizeImage = imageSize;

    unsigned char *palette = nullptr;

    if (channels == 1)
    {
        palette = new unsigned char[256 * 4];
        for (int i = 0; i < 256; ++i)
        {
            *(palette + i * 4 + 0) = static_cast<unsigned char>(i); // B
            *(palette + i * 4 + 1) = static_cast<unsigned char>(i); // G
            *(palette + i * 4 + 2) = static_cast<unsigned char>(i); // R
            *(palette + i * 4 + 3) = 0;                             // reserved
        }
        fileHeader.bfOffBits += 256 * 4;
        fileHeader.bfSize = fileHeader.bfOffBits + imageSize;
        infoHeader.biClrUsed = 256;
    }
    else
    {
        fileHeader.bfSize = fileHeader.bfOffBits + imageSize;
    }

    std::ofstream out(filename, std::ios::binary);
    if (!out)
    {
        delete[] palette;
        return false;
    }

    out.write(reinterpret_cast<const char *>(&fileHeader), sizeof(fileHeader));
    out.write(reinterpret_cast<const char *>(&infoHeader), sizeof(infoHeader));

    if (palette)
    {
        out.write(reinterpret_cast<const char *>(palette), 256 * 4);
    }

    unsigned char *row = new unsigned char[rowPaddedBytes];
    unsigned char *fullImageBuffer = new unsigned char[rowPaddedBytes * height];
#pragma omp parallel for schedule(static)
    for (int y = 0; y < height; ++y)
    {
        const unsigned char *srcLine = src + (height - 1 - y) * width * channels;
        unsigned char *dstLine = fullImageBuffer + y * rowPaddedBytes;

        if (channels == 1)
        {
            std::memcpy(dstLine, srcLine, width);
        }
        else if (channels == 3)
        {
            for (int x = 0; x < width; ++x)
            {
                *(dstLine + x * 3 + 0) = *(srcLine + x * 3 + 0);
                *(dstLine + x * 3 + 1) = *(srcLine + x * 3 + 1);
                *(dstLine + x * 3 + 2) = *(srcLine + x * 3 + 2);
            }
        }
        else if (channels == 4)
        {
            for (int x = 0; x < width; ++x)
            {
                *(dstLine + x * 4 + 0) = *(srcLine + x * 4 + 0);
                *(dstLine + x * 4 + 1) = *(srcLine + x * 4 + 1);
                *(dstLine + x * 4 + 2) = *(srcLine + x * 4 + 2);
                *(dstLine + x * 4 + 3) = *(srcLine + x * 4 + 3);
            }
        }
    }
    out.write(reinterpret_cast<const char *>(fullImageBuffer), rowPaddedBytes * height);
    delete[] fullImageBuffer;
    delete[] row;
    delete[] palette;

    return true;
}
