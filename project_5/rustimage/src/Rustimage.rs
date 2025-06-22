use image::{imageops, open, DynamicImage, GenericImageView, Rgba};
use rayon::prelude::*;
use std::io;
use std::sync::{Arc, Mutex};
use std::time::Instant;
use std::arch::aarch64::*;
use std::thread;
use std::time::Duration;

#[target_feature(enable = "neon")]
unsafe fn adjust_brightness_contrast(img: &DynamicImage, brightness: i32, contrast: f32) -> DynamicImage {
    let (w, h) = img.dimensions();
    let mut result = image::ImageBuffer::new(w, h);
    let src = img.to_rgba8();
    let contrast_vec = vdupq_n_f32(contrast);
    let brightness_vec = vdupq_n_f32(brightness as f32);

    result
        .par_chunks_mut(4 * w as usize)
        .zip(src.par_chunks(4 * w as usize))
        .for_each(|(row_dst, row_src)| {
            for (chunk_dst, chunk_src) in row_dst.chunks_mut(16).zip(row_src.chunks(16)) {
                unsafe {
                    let pixel_u8 = vld1q_u8(chunk_src.as_ptr());
                    let pixel_lo = vmovl_u8(vget_low_u8(pixel_u8));
                    let pixel_hi = vmovl_u8(vget_high_u8(pixel_u8));

                    let lo_f32 = vcvtq_f32_u32(vmovl_u16(vget_low_u16(pixel_lo)));
                    let hi_f32 = vcvtq_f32_u32(vmovl_u16(vget_low_u16(pixel_hi)));

                    let lo_adj = vmlaq_f32(brightness_vec, lo_f32, contrast_vec);
                    let hi_adj = vmlaq_f32(brightness_vec, hi_f32, contrast_vec);

                    let lo_u8 = vqmovn_u16(vcombine_u16(vqmovn_u32(vcvtq_u32_f32(lo_adj)), vdup_n_u16(0)));
                    let hi_u8 = vqmovn_u16(vcombine_u16(vqmovn_u32(vcvtq_u32_f32(hi_adj)), vdup_n_u16(0)));
                    let result_vec = vcombine_u8(lo_u8, hi_u8);

                    vst1q_u8(chunk_dst.as_mut_ptr(), result_vec);
                }
            }
        });

    DynamicImage::ImageRgba8(result)
}
#[target_feature(enable = "neon")]
unsafe fn blend_images(img1: &DynamicImage, img2: &DynamicImage) -> DynamicImage {
    let (w, h) = img1.dimensions();
    let mut result = image::ImageBuffer::new(w, h);
    let buf1 = img1.to_rgba8();
    let buf2 = img2.to_rgba8();

    result
        .par_chunks_mut(16)
        .zip(buf1.par_chunks(16).zip(buf2.par_chunks(16)))
        .for_each(|(dst_chunk, (chunk1, chunk2))| {
            unsafe {
                let va = vld1q_u8(chunk1.as_ptr());
                let vb = vld1q_u8(chunk2.as_ptr());

                let a_lo = vmovl_u8(vget_low_u8(va));
                let a_hi = vmovl_u8(vget_high_u8(va));
                let b_lo = vmovl_u8(vget_low_u8(vb));
                let b_hi = vmovl_u8(vget_high_u8(vb));

                let avg_lo = vshrq_n_u16(vaddq_u16(vaddq_u16(a_lo, b_lo), vdupq_n_u16(1)), 1);
                let avg_hi = vshrq_n_u16(vaddq_u16(vaddq_u16(a_hi, b_hi), vdupq_n_u16(1)), 1);

                let out_lo = vqmovn_u16(avg_lo);
                let out_hi = vqmovn_u16(avg_hi);
                let blended = vcombine_u8(out_lo, out_hi);

                vst1q_u8(dst_chunk.as_mut_ptr(), blended);
            }
        });

    DynamicImage::ImageRgba8(result)
}


fn resize_image(img: &DynamicImage, new_w: u32, new_h: u32) -> DynamicImage {
    img.resize_exact(new_w, new_h, imageops::FilterType::Triangle)
}

fn main() {
    let mut input1 = String::new();
    let mut operation = String::new();

    // 获取输入路径和操作
    println!("请输入第一张图像路径：");
    io::stdin().read_line(&mut input1).expect("读取路径失败");

    // 加上 ../ 作为路径前缀
    let input1 = format!("../{}", input1.trim());

    // 图像读取计时
    let img_read_start = Instant::now();
    let img1 = open(input1).expect("无法打开图像文件");
    let img_read_duration = img_read_start.elapsed().as_secs_f64() * 1000.0; // 转换为毫秒
    println!("图像读取耗时: {:.3}毫秒", img_read_duration);

    println!("请输入操作类型（adjust / blend / resize）：");
    io::stdin()
        .read_line(&mut operation)
        .expect("读取操作类型失败");
    let operation = operation.trim();
    let img2 = img1.clone(); // 与自己进行融合
                             
    let img_process_start = Instant::now();
    let result = match operation {
        "adjust" => {
            let brightness = 40; // 亮度调整固定为 40
            let contrast = 1.5; // 对比度调整固定为 1.5
            unsafe{adjust_brightness_contrast(&img1, brightness, contrast)}
        }
        "blend" => unsafe{blend_images(&img1, &img2)},
        "resize" => {
            let new_w = img1.width() * 2; // 新宽度是原来的两倍
            let new_h = img1.height() * 2; // 新高度是原来的两倍
            resize_image(&img1, new_w, new_h)
        }
        _ => panic!("不支持的操作类型！"),
    };
    let img_process_duration = img_process_start.elapsed().as_secs_f64() * 1000.0; // 转换为毫秒
    println!("图像处理耗时: {:.3}毫秒", img_process_duration);

    // 图像保存计时
    let img_save_start = Instant::now();
    result.save("output.bmp").expect("保存图像失败");
    let img_save_duration = img_save_start.elapsed().as_secs_f64() * 1000.0; // 转换为毫秒
    println!("图像保存耗时: {:.3}毫秒", img_save_duration);

    let garbage = "../garbage.bmp";
    let img2 = open(garbage).expect("无法打开图像文件");
    unsafe{adjust_brightness_contrast(&img2, 2, 2.0)};
    img2.save("garbage_output.bmp").expect("保存图像失败");
    thread::sleep(Duration::from_millis(500));
}
