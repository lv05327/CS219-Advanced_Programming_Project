#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#pragma pack(push, 1) // 确保结构体紧凑排列
typedef struct
{
    char signature[2]; // "BM"
    int file_size;     // 文件大小
    short reserved1;   // 保留
    short reserved2;   // 保留
    int offset;        // 像素数据偏移量
} BMPFileHeader;

typedef struct
{
    int header_size;      // 信息头大小
    int width;            // 图像宽度
    int height;           // 图像高度
    short planes;         // 颜色平面数
    short bits_per_pixel; // 每像素位数
    int compression;      // 压缩方式
    int image_size;       // 图像数据大小
    int x_pixels_per_m;   // 水平分辨率
    int y_pixels_per_m;   // 垂直分辨率
    int colors_used;      // 使用的颜色数
    int important_colors; // 重要颜色数
} BMPInfoHeader;

typedef struct
{
    BMPFileHeader file_header;
    BMPInfoHeader info_header;
    unsigned char *pixel_data; // 像素数据
} BMPImage;
#pragma pack(pop)

// 读取BMP图像
BMPImage *read_bmp(const char *filename)
{
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        fprintf(stderr, "无法打开文件: %s\n", filename);
        return NULL;
    }

    BMPImage *image = (BMPImage *)malloc(sizeof(BMPImage));
    if (!image)
    {
        fclose(file);
        fprintf(stderr, "内存分配失败\n");
        return NULL;
    }

    // 读取文件头
    if (fread(&image->file_header, sizeof(BMPFileHeader), 1, file) != 1)
    {
        fclose(file);
        free(image);
        fprintf(stderr, "读取文件头失败\n");
        return NULL;
    }

    // 检查是否是BMP文件
    if (image->file_header.signature[0] != 'B' || image->file_header.signature[1] != 'M')
    {
        fclose(file);
        free(image);
        fprintf(stderr, "不是有效的BMP文件\n");
        return NULL;
    }

    // 读取信息头
    if (fread(&image->info_header, sizeof(BMPInfoHeader), 1, file) != 1)
    {
        fclose(file);
        free(image);
        fprintf(stderr, "读取信息头失败\n");
        return NULL;
    }

    // 检查是否是24位无压缩BMP
    if (image->info_header.bits_per_pixel != 24 || image->info_header.compression != 0)
    {
        fclose(file);
        free(image);
        fprintf(stderr, "只支持24位无压缩BMP文件\n");
        return NULL;
    }

    // 检查图像大小是否超出处理范围
    if ((long long)image->info_header.width *
            (long long)image->info_header.height * 3 >
        INT_MAX)
    {
        fclose(file);
        free(image);
        fprintf(stderr, "图片像素过大，超出处理范围\n");
        return NULL;
    }

    // 计算每行的实际字节数（包括填充位）
    int row_size = (image->info_header.width * 3 + 3) & ~3;
    int pixel_data_size = row_size * image->info_header.height;

    // 分配像素数据内存
    image->pixel_data = (unsigned char *)malloc(pixel_data_size);
    if (!image->pixel_data)
    {
        fclose(file);
        free(image);
        fprintf(stderr, "像素数据内存分配失败\n");
        return NULL;
    }

    // 移动到像素数据位置
    fseek(file, image->file_header.offset, SEEK_SET);

    // 读取像素数据
    if (fread(image->pixel_data, 1, pixel_data_size, file) != pixel_data_size)
    {
        fclose(file);
        free(image->pixel_data);
        free(image);
        fprintf(stderr, "读取像素数据失败\n");
        return NULL;
    }

    fclose(file);
    return image;
}

// 保存BMP图像
int write_bmp(const char *filename, BMPImage *image)
{
    FILE *file = fopen(filename, "wb");
    if (!file)
    {
        fprintf(stderr, "无法创建文件: %s\n", filename);
        return 0;
    }

    // 写入文件头
    if (fwrite(&image->file_header, sizeof(BMPFileHeader), 1, file) != 1)
    {
        fclose(file);
        fprintf(stderr, "写入文件头失败\n");
        return 0;
    }

    // 写入信息头
    if (fwrite(&image->info_header, sizeof(BMPInfoHeader), 1, file) != 1)
    {
        fclose(file);
        fprintf(stderr, "写入信息头失败\n");
        return 0;
    }

    // 计算每行实际字节数（包括填充位）
    int row_size = (image->info_header.width * 3 + 3) & ~3;
    int pixel_data_size = row_size * image->info_header.height;

    // 写入像素数据（包括填充位）
    if (fwrite(image->pixel_data, 1, pixel_data_size, file) != pixel_data_size)
    {
        fclose(file);
        fprintf(stderr, "写入像素数据失败\n");
        return 0;
    }

    fclose(file);
    return 1;
}

// 释放BMP图像内存
void free_bmp(BMPImage *image)
{
    if (image)
    {
        free(image->pixel_data);
        free(image);
    }
}

// 调整亮度
void adjust_brightness(BMPImage *image, int value)
{
    int row_size = (image->info_header.width * 3 + 3) & ~3; // 每行实际字节数（包括填充位）
    int row_width = image->info_header.width * 3;           // 每行有效像素数据字节数

    unsigned char *pixel = image->pixel_data;

    for (int y = 0; y < image->info_header.height; y++)
    {
        unsigned char *row_start = pixel; // 当前行的起始位置
        for (int x = 0; x < row_width; x++)
        {
            int new_value = *pixel + value;
            *pixel = (new_value > 255) ? 255 : ((new_value < 0) ? 0 : new_value);
            pixel++;
        }
        pixel = row_start + row_size; // 跳过填充位，移动到下一行
    }
}

// 混合两幅图像
void blend_images(BMPImage *img1, BMPImage *img2)
{
    // 检查图像尺寸是否相同
    if (img1->info_header.width != img2->info_header.width ||
        img1->info_header.height != img2->info_header.height)
    {
        fprintf(stderr, "图像尺寸不匹配\n");
        return;
    }

    unsigned char *p1 = img1->pixel_data;
    unsigned char *p2 = img2->pixel_data;
    unsigned char *end = p1 + img1->info_header.width * img1->info_header.height * 3;

    while (p1 < end)
    {
        int sum = (int)*p1 + (int)*p2;
        *p1 = (unsigned char)(sum >> 1);
        p1++;
        p2++;
    }
}

BMPImage *crop_image(BMPImage *image, int x, int y, int width, int height)
{
    // 检查剪裁范围是否有效
    if (x < 0 || y < 0 || width <= 0 || height <= 0 || x + width > image->info_header.width || y + height > image->info_header.height)
    {
        fprintf(stderr, "剪裁范围无效\n");
        return NULL;
    }

    // 创建新图像
    BMPImage *result = (BMPImage *)malloc(sizeof(BMPImage));
    if (!result)
        return NULL;

    // 复制头部信息并修改尺寸
    memcpy(&result->file_header, &image->file_header, sizeof(BMPFileHeader));
    memcpy(&result->info_header, &image->info_header, sizeof(BMPInfoHeader));
    result->info_header.width = width;
    result->info_header.height = height;

    // 计算新图像的行大小（包括填充位）
    int new_row_size = (width * 3 + 3) & ~3;
    result->info_header.image_size = new_row_size * height;
    result->file_header.file_size = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + result->info_header.image_size;

    // 分配新的像素数据内存
    result->pixel_data = (unsigned char *)malloc(result->info_header.image_size);
    if (!result->pixel_data)
    {
        free(result);
        return NULL;
    }

    // 计算原图行大小
    int old_row_size = (image->info_header.width * 3 + 3) & ~3;
    unsigned char *src = image->pixel_data + y * old_row_size + x * 3;
    unsigned char *dst = result->pixel_data;

    // 复制像素数据（自动保留填充位的0值）
    for (int i = 0; i < height; i++)
    {
        memcpy(dst, src, width * 3);
        memset(dst + width * 3, 0, new_row_size - width * 3);
        src += old_row_size;
        dst += new_row_size;
    }
    return result;
}

BMPImage *resize_image(BMPImage *image, int new_width, int new_height)
{
    if (new_width <= 0 || new_height <= 0)
    {
        fprintf(stderr, "无效的目标尺寸\n");
        return NULL;
    }

    // 检查目标图像大小是否超出处理范围
    if ((long long)new_width * (long long)new_height * 3 > INT_MAX)
    {
        fprintf(stderr, "目标的图像太大了\n");
        return NULL;
    }

    // 创建新图像
    BMPImage *result = (BMPImage *)malloc(sizeof(BMPImage));
    if (!result)
        return NULL;

    // 复制并修改头部信息
    memcpy(&result->file_header, &image->file_header, sizeof(BMPFileHeader));
    memcpy(&result->info_header, &image->info_header, sizeof(BMPInfoHeader));
    result->info_header.width = new_width;
    result->info_header.height = new_height;

    // 计算新图像的行大小（包括填充位）
    int new_row_size = (new_width * 3 + 3) & ~3;
    result->info_header.image_size = new_row_size * new_height;
    result->file_header.file_size = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + result->info_header.image_size;

    // 分配新的像素数据内存
    result->pixel_data = (unsigned char *)malloc(result->info_header.image_size);
    if (!result->pixel_data)
    {
        free(result);
        return NULL;
    }

    unsigned char *src = image->pixel_data;
    unsigned char *dst = result->pixel_data;
    int old_width = image->info_header.width;
    int old_height = image->info_header.height;

    // 计算缩放比例
    float x_ratio = (float)old_width / new_width;
    float y_ratio = (float)old_height / new_height;

    // 计算原图的行大小（包括填充位）
    int old_row_size = (old_width * 3 + 3) & ~3;

    // 双线性插值算法
    for (int y = 0; y < new_height; y++)
    {
        float src_y = y * y_ratio;
        int y1 = (int)src_y;
        int y2 = (y1 + 1 < old_height) ? y1 + 1 : y1;
        float y_diff = src_y - y1;

        unsigned char *dst_row = dst + y * new_row_size;  // 当前目标行的起始位置

        for (int x = 0; x < new_width; x++)
        {
            float src_x = x * x_ratio;
            int x1 = (int)src_x;
            int x2 = (x1 + 1 < old_width) ? x1 + 1 : x1;
            float x_diff = src_x - x1;

            // 获取四个像素点的指针
            unsigned char *p11 = src + y1 * old_row_size + x1 * 3;
            unsigned char *p12 = src + y1 * old_row_size + x2 * 3;
            unsigned char *p21 = src + y2 * old_row_size + x1 * 3;
            unsigned char *p22 = src + y2 * old_row_size + x2 * 3;

            // 对 R、G、B 三个通道分别进行插值
            unsigned char *dst_pixel = dst_row + x * 3;
            while (dst_pixel < dst_row + x * 3 + 3)
            {
                float r1 = (*p11) * (1 - x_diff) + (*p12) * x_diff;
                float r2 = (*p21) * (1 - x_diff) + (*p22) * x_diff;
                *dst_pixel = (unsigned char)(r1 * (1 - y_diff) + r2 * y_diff);
                p11++;
                p12++;
                p21++;
                p22++;
                dst_pixel++;
            }
        }

        // 填充当前行的尾部
        memset(dst_row + new_width * 3, 0, new_row_size - new_width * 3);
    }

    return result;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        return 1;
    }

    char *input_file1 = NULL;
    char *input_file2 = NULL;
    char *output_file = NULL;
    char *operation = NULL;
    int value = 0;
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
    int new_width = 0;
    int new_height = 0;

    // 解析命令行参数
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-i") == 0 && i + 1 < argc)
        {
            if (!input_file1)
            {
                input_file1 = argv[++i];
            }
            else
            {
                input_file2 = argv[++i];
            }
        }
        else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc)
        {
            output_file = argv[++i];
        }
        else if (strcmp(argv[i], "-op") == 0 && i + 1 < argc)
        {
            operation = argv[++i];
            if (strcmp(operation, "add") == 0 && i + 1 < argc)
            {
                value = atoi(argv[++i]);
            }
            else if (strcmp(operation, "crop") == 0 && i + 4 < argc)
            {
                // 读取剪裁参数
                x = atoi(argv[++i]);
                y = atoi(argv[++i]);
                width = atoi(argv[++i]);
                height = atoi(argv[++i]);
            }
            else if (strcmp(operation, "resize") == 0 && i + 2 < argc)
            {
                // 读取缩放参数
                new_width = atoi(argv[++i]);
                new_height = atoi(argv[++i]);
            }
        }
    }

    if (!input_file1 || !output_file || !operation)
    {
        return 1;
    }

    BMPImage *image1 = read_bmp(input_file1);
    if (!image1)
    {
        return 1;
    }

    if (strcmp(operation, "add") == 0)
    {
        adjust_brightness(image1, value);
        if (!write_bmp(output_file, image1))
        {
            free_bmp(image1);
            return 1;
        }
    }
    else if (strcmp(operation, "average") == 0)
    {
        if (!input_file2)
        {
            printf("需要第二个输入文件用于混合\n");
            free_bmp(image1);
            return 1;
        }
        BMPImage *image2 = read_bmp(input_file2);
        if (!image2)
        {
            free_bmp(image1);
            return 1;
        }

        // 直接在image1上进行混合
        blend_images(image1, image2);

        if (!write_bmp(output_file, image1))
        {
            free_bmp(image1);
            free_bmp(image2);
            return 1;
        }
        free_bmp(image2);
    }
    else if (strcmp(operation, "crop") == 0)
    {
        BMPImage *cropped = crop_image(image1, x, y, width, height);
        if (!cropped)
        {
            free_bmp(image1);
            return 1;
        }
        if (!write_bmp(output_file, cropped))
        {
            free_bmp(image1);
            free_bmp(cropped);
            return 1;
        }
        free_bmp(cropped);
    }
    else if (strcmp(operation, "resize") == 0)
    {
        BMPImage *resized = resize_image(image1, new_width, new_height);
        if (!resized)
        {
            free_bmp(image1);
            return 1;
        }
        if (!write_bmp(output_file, resized))
        {
            free_bmp(image1);
            free_bmp(resized);
            return 1;
        }
        free_bmp(resized);
    }
    else
    {
        printf("您输入了不支持或错误的操作\n");
        printf("使用格式:\n");
        printf("调整亮度: ./bmpedit -i input.bmp -o output.bmp -op add value\n");
        printf("图像混合: ./bmpedit -i input1.bmp -i input2.bmp -o output.bmp -op average\n");
        printf("图像剪裁: ./bmpedit -i input.bmp -o output.bmp -op crop x y width height\n");
        printf("图像缩放: ./bmpedit -i input.bmp -o output.bmp -op resize width height\n");
        free_bmp(image1);
        return 1;
    }

    free_bmp(image1);
    printf("操作完成\n");
    return 0;
}