#include <arm_neon.h>
#include <omp.h>
#include <cmath>
#include <Mat.h>
#include <iostream>
#include <cstring>

template <typename T>
constexpr T clamp(T value, T minVal, T maxVal)
{
    return value < minVal ? minVal : (value > maxVal ? maxVal : value);
}

Mat Mat::adjustBrightnessContrast(int brightness, float contrast) const {
    Mat result(width, height, channels);
    const uint8_t* src = data.get();
    uint8_t* dst = result.getWritableData();
    int total = width * height * channels;

    const int alpha_q8 = static_cast<int>(contrast * 256 + 0.5f);
    const int beta = brightness;

#pragma omp parallel for
    for (int i = 0; i <= total - 16; i += 16) {
        const uint8_t* sp = src + i;
        uint8_t* dp = dst + i;

        // 加载原始像素
        uint8x16_t pixels = vld1q_u8(sp);

        // 解包为 16 位无符号整数
        uint16x8_t lo = vmovl_u8(vget_low_u8(pixels));
        uint16x8_t hi = vmovl_u8(vget_high_u8(pixels));

        // 解包为 32 位整数
        uint32x4_t lo0 = vmovl_u16(vget_low_u16(lo));
        uint32x4_t lo1 = vmovl_u16(vget_high_u16(lo));
        uint32x4_t hi0 = vmovl_u16(vget_low_u16(hi));
        uint32x4_t hi1 = vmovl_u16(vget_high_u16(hi));

        // scale: (value * alpha_q8 + beta) >> 8
        uint32x4_t lo0_adj = vmlaq_n_u32(vdupq_n_u32(beta), lo0, alpha_q8);
        uint32x4_t lo1_adj = vmlaq_n_u32(vdupq_n_u32(beta), lo1, alpha_q8);
        uint32x4_t hi0_adj = vmlaq_n_u32(vdupq_n_u32(beta), hi0, alpha_q8);
        uint32x4_t hi1_adj = vmlaq_n_u32(vdupq_n_u32(beta), hi1, alpha_q8);

        // 缩放回 8 位，自动饱和
        uint16x4_t r0 = vqshrn_n_u32(lo0_adj, 8);
        uint16x4_t r1 = vqshrn_n_u32(lo1_adj, 8);
        uint16x4_t r2 = vqshrn_n_u32(hi0_adj, 8);
        uint16x4_t r3 = vqshrn_n_u32(hi1_adj, 8);

        // 打包结果
        uint8x8_t result_lo = vqmovn_u16(vcombine_u16(r0, r1));
        uint8x8_t result_hi = vqmovn_u16(vcombine_u16(r2, r3));
        vst1q_u8(dp, vcombine_u8(result_lo, result_hi));
    }

    // 处理剩余像素（非16对齐）
    for (int i = (total / 16) * 16; i < total; ++i) {
        int val = ((src[i] * alpha_q8) >> 8) + beta;
        dst[i] = static_cast<uint8_t>(clamp(val, 0, 255));
    }

    return result;
}


Mat blend(const Mat &a, const Mat &b)
{
    if (a.getWidth() != b.getWidth() || a.getHeight() != b.getHeight() || a.getChannels() != b.getChannels())
        throw std::runtime_error("blend: image dimensions must match");

    int total = a.getWidth() * a.getHeight() * a.getChannels();
    Mat result(a.getWidth(), a.getHeight(), a.getChannels());

    const uint8_t *dataA = a.getData();
    const uint8_t *dataB = b.getData();
    uint8_t *dst = result.getWritableData();

    const int simdWidth = 16;

#pragma omp parallel for schedule(static)
    for (int i = 0; i <= total - simdWidth; i += simdWidth)
    {
        const uint8_t* pa = dataA + i;
        const uint8_t* pb = dataB + i;
        uint8_t* pd = dst + i;

        uint8x16_t va = vld1q_u8(pa);
        uint8x16_t vb = vld1q_u8(pb);

        uint16x8_t a_lo = vmovl_u8(vget_low_u8(va));
        uint16x8_t a_hi = vmovl_u8(vget_high_u8(va));
        uint16x8_t b_lo = vmovl_u8(vget_low_u8(vb));
        uint16x8_t b_hi = vmovl_u8(vget_high_u8(vb));

        uint16x8_t sum_lo = vshrq_n_u16(vaddq_u16(vaddq_u16(a_lo, b_lo), vdupq_n_u16(1)), 1);
        uint16x8_t sum_hi = vshrq_n_u16(vaddq_u16(vaddq_u16(a_hi, b_hi), vdupq_n_u16(1)), 1);

        uint8x8_t out_lo = vqmovn_u16(sum_lo);
        uint8x8_t out_hi = vqmovn_u16(sum_hi);
        vst1q_u8(pd, vcombine_u8(out_lo, out_hi));
    }

#pragma omp parallel for
    for (int i = (total / simdWidth) * simdWidth; i < total; ++i)
    {
        dst[i] = static_cast<uint8_t>((dataA[i] + dataB[i] + 1) / 2);
    }

    return result;
}


Mat Mat::toGrayscale() const
{
    if (channels < 3)
        throw std::runtime_error("toGrayscale requires at least 3 channels.");

    Mat result(width, height, 1);
    const unsigned char *src = data.get();
    unsigned char *dst = result.getWritableData();
    int totalPixels = width * height;

    const float32x4_t fR = vdupq_n_f32(0.299f);
    const float32x4_t fG = vdupq_n_f32(0.587f);
    const float32x4_t fB = vdupq_n_f32(0.114f);

#pragma omp parallel for
    for (int i = 0; i <= totalPixels - 16; i += 16)
    {
        const uint8_t* s = src + i * channels;
        uint8x16x3_t bgr = vld3q_u8(s);

        uint16x8_t b_lo = vmovl_u8(vget_low_u8(bgr.val[0]));
        uint16x8_t g_lo = vmovl_u8(vget_low_u8(bgr.val[1]));
        uint16x8_t r_lo = vmovl_u8(vget_low_u8(bgr.val[2]));
        uint16x8_t b_hi = vmovl_u8(vget_high_u8(bgr.val[0]));
        uint16x8_t g_hi = vmovl_u8(vget_high_u8(bgr.val[1]));
        uint16x8_t r_hi = vmovl_u8(vget_high_u8(bgr.val[2]));

        float32x4_t fb0 = vcvtq_f32_u32(vmovl_u16(vget_low_u16(b_lo)));
        float32x4_t fg0 = vcvtq_f32_u32(vmovl_u16(vget_low_u16(g_lo)));
        float32x4_t fr0 = vcvtq_f32_u32(vmovl_u16(vget_low_u16(r_lo)));
        float32x4_t fb1 = vcvtq_f32_u32(vmovl_u16(vget_high_u16(b_lo)));
        float32x4_t fg1 = vcvtq_f32_u32(vmovl_u16(vget_high_u16(g_lo)));
        float32x4_t fr1 = vcvtq_f32_u32(vmovl_u16(vget_high_u16(r_lo)));

        float32x4_t fb2 = vcvtq_f32_u32(vmovl_u16(vget_low_u16(b_hi)));
        float32x4_t fg2 = vcvtq_f32_u32(vmovl_u16(vget_low_u16(g_hi)));
        float32x4_t fr2 = vcvtq_f32_u32(vmovl_u16(vget_low_u16(r_hi)));
        float32x4_t fb3 = vcvtq_f32_u32(vmovl_u16(vget_high_u16(b_hi)));
        float32x4_t fg3 = vcvtq_f32_u32(vmovl_u16(vget_high_u16(g_hi)));
        float32x4_t fr3 = vcvtq_f32_u32(vmovl_u16(vget_high_u16(r_hi)));

        float32x4_t gray0 = vmlaq_f32(vmlaq_f32(vmulq_f32(fr0, fR), fg0, fG), fb0, fB);
        float32x4_t gray1 = vmlaq_f32(vmlaq_f32(vmulq_f32(fr1, fR), fg1, fG), fb1, fB);
        float32x4_t gray2 = vmlaq_f32(vmlaq_f32(vmulq_f32(fr2, fR), fg2, fG), fb2, fB);
        float32x4_t gray3 = vmlaq_f32(vmlaq_f32(vmulq_f32(fr3, fR), fg3, fG), fb3, fB);

        uint16x4_t g0 = vmovn_u32(vcvtq_u32_f32(gray0));
        uint16x4_t g1 = vmovn_u32(vcvtq_u32_f32(gray1));
        uint16x4_t g2 = vmovn_u32(vcvtq_u32_f32(gray2));
        uint16x4_t g3 = vmovn_u32(vcvtq_u32_f32(gray3));

        uint8x8_t gray_lo = vmovn_u16(vcombine_u16(g0, g1));
        uint8x8_t gray_hi = vmovn_u16(vcombine_u16(g2, g3));
        uint8x16_t gray_final = vcombine_u8(gray_lo, gray_hi);

        vst1q_u8(dst + i, gray_final);
    }

#pragma omp parallel for
    for (int i = (totalPixels / 16) * 16; i < totalPixels; ++i)
    {
        const unsigned char *px = src + i * channels;
        float gray = px[2] * 0.299f + px[1] * 0.587f + px[0] * 0.114f;
        dst[i] = static_cast<unsigned char>(std::round(gray));
    }

    return result;
}



Mat Mat::resize(int new_width, int new_height) const
{
    if (new_width <= 0 || new_height <= 0)
        throw std::invalid_argument("Invalid dimensions for resize.");

    if (channels != 1 && channels != 3)
        throw std::runtime_error("resize_NEON only supports 1 or 3 channels");

    Mat result(new_width, new_height, channels);
    const unsigned char* src = data.get();
    unsigned char* dst = result.getWritableData();

    float x_ratio = static_cast<float>(width) / new_width;
    float y_ratio = static_cast<float>(height) / new_height;

#pragma omp parallel for schedule(static)
    for (int y = 0; y < new_height; ++y)
    {
        float fy = y * y_ratio;
        int y1 = static_cast<int>(fy);
        int y2 = std::min(y1 + 1, height - 1);
        float dy = fy - y1;
        float32x4_t dyv = vdupq_n_f32(dy);
        float32x4_t onedy = vdupq_n_f32(1.0f - dy);

        for (int x = 0; x <= new_width - 4; x += 4)
        {
            float32x4_t fxv = {
                (x + 0) * x_ratio,
                (x + 1) * x_ratio,
                (x + 2) * x_ratio,
                (x + 3) * x_ratio
            };

            int x1s[4], x2s[4];
            float dx[4];
            for (int i = 0; i < 4; ++i)
            {
                x1s[i] = static_cast<int>(fxv[i]);
                x2s[i] = std::min(x1s[i] + 1, width - 1);
                dx[i] = fxv[i] - x1s[i];
            }

            float32x4_t dxv = vld1q_f32(dx);
            float32x4_t onedx = vsubq_f32(vdupq_n_f32(1.0f), dxv);

            for (int c = 0; c < channels; ++c)
            {
                float32x4_t v1 = {
                    static_cast<float>(src[(y1 * width + x1s[0]) * channels + c]),
                    static_cast<float>(src[(y1 * width + x1s[1]) * channels + c]),
                    static_cast<float>(src[(y1 * width + x1s[2]) * channels + c]),
                    static_cast<float>(src[(y1 * width + x1s[3]) * channels + c])
                };

                float32x4_t v2 = {
                    static_cast<float>(src[(y1 * width + x2s[0]) * channels + c]),
                    static_cast<float>(src[(y1 * width + x2s[1]) * channels + c]),
                    static_cast<float>(src[(y1 * width + x2s[2]) * channels + c]),
                    static_cast<float>(src[(y1 * width + x2s[3]) * channels + c])
                };

                float32x4_t v3 = {
                    static_cast<float>(src[(y2 * width + x1s[0]) * channels + c]),
                    static_cast<float>(src[(y2 * width + x1s[1]) * channels + c]),
                    static_cast<float>(src[(y2 * width + x1s[2]) * channels + c]),
                    static_cast<float>(src[(y2 * width + x1s[3]) * channels + c])
                };

                float32x4_t v4 = {
                    static_cast<float>(src[(y2 * width + x2s[0]) * channels + c]),
                    static_cast<float>(src[(y2 * width + x2s[1]) * channels + c]),
                    static_cast<float>(src[(y2 * width + x2s[2]) * channels + c]),
                    static_cast<float>(src[(y2 * width + x2s[3]) * channels + c])
                };

                float32x4_t top = vmlaq_f32(vmulq_f32(onedx, v1), dxv, v2);
                float32x4_t bot = vmlaq_f32(vmulq_f32(onedx, v3), dxv, v4);
                float32x4_t val = vmlaq_f32(vmulq_f32(onedy, top), dyv, bot);

                float out[4];
                vst1q_f32(out, val);

                unsigned char* dst_ptr = dst + (y * new_width + x) * channels + c;
                for (int i = 0; i < 4; ++i)
                {
                    dst_ptr[i * channels] = static_cast<unsigned char>(clamp(std::round(out[i]), 0.0f, 255.0f));
                }
            }
        }
    }

    return result;
}
Mat Mat::flip() const
{
    Mat result(width, height, channels);
    const unsigned char *src = data.get();
    unsigned char *dst = result.getWritableData();
    int rowSize = width * channels;

    char mode;
    do {
        std::cout << "Flip image. Enter 'H' for horizontal or 'V' for vertical flip: ";
        std::cin >> mode;
    } while (mode != 'H' && mode != 'h' && mode != 'V' && mode != 'v');

    if (mode == 'V' || mode == 'v')
    {
#pragma omp parallel for
        for (int y = 0; y < height; ++y)
        {
            const unsigned char *srcRow = src + y * rowSize;
            unsigned char *dstRow = dst + (height - 1 - y) * rowSize;
            std::memcpy(dstRow, srcRow, rowSize);
        }
    }
    else
    {
#pragma omp parallel for
        for (int y = 0; y < height; ++y)
        {
            const unsigned char *srcRow = src + y * rowSize;
            unsigned char *dstRow = dst + y * rowSize;
            for (int x = 0; x < width; ++x)
            {
                const unsigned char *srcPixel = srcRow + (width - 1 - x) * channels;
                unsigned char *dstPixel = dstRow + x * channels;
                for (int c = 0; c < channels; ++c)
                {
                    dstPixel[c] = srcPixel[c];
                }
            }
        }
    }

    return result;
}

Mat Mat::crop(int x, int y, int new_width, int new_height) const
{
    if (x < 0 || y < 0 || new_width <= 0 || new_height <= 0 ||
        x + new_width > width || y + new_height > height)
        throw std::runtime_error("Invalid crop parameters");

    Mat result(new_width, new_height, channels);
    const int srcRowBytes = width * channels;
    const int dstRowBytes = new_width * channels;

    const unsigned char *srcData = data.get();
    unsigned char *dstData = result.getWritableData();

#pragma omp parallel for
    for (int row = 0; row < new_height; ++row)
    {
        const unsigned char *srcLine = srcData + (y + row) * srcRowBytes + x * channels;
        unsigned char *dstLine = dstData + row * dstRowBytes;
        std::memcpy(dstLine, srcLine, dstRowBytes);
    }

    return result;
}
