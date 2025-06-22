import cv2
import numpy as np
import time
import sys
import gc
from memory_profiler import profile
@profile
def main():
# 1. 修改用户输入：图像路径 + 操作类型
    image_path = "campus.bmp"
    operation = "blend"

# 2. 加载图像
    start_0 = time.perf_counter()
    img = cv2.imread(image_path)
    end_0 = time.perf_counter()
    elapsed_ms = (end_0 - start_0) * 1000
    print(f"读入耗时: {elapsed_ms:.3f} 毫秒")

    if img is None:
        print("图像加载失败！请检查路径。")
        sys.exit(1)

    gc.disable()#计时前关闭gc机制，有利于测量时间的稳定
# 4. 计时开始
    start_1 = time.perf_counter()

# 5. 执行图像处理操作
    if operation == "adjust":
        result = cv2.convertScaleAbs(img, alpha=1.5, beta=40)
    elif operation == "blend":
        result = cv2.addWeighted(img, 0.5, img, 0.5, 0)
    elif operation == "resize":
        result = cv2.resize(img, (img.shape[1] * 2, img.shape[0] * 2), interpolation=cv2.INTER_LINEAR)
    else:
        print("不支持的操作类型！")
        sys.exit(1)

# 6. 计时结束
    end_1 = time.perf_counter()
    gc.enable()
    start_2 = time.perf_counter()
    cv2.imwrite("output.bmp", result)
    end_2 = time.perf_counter()
    elapsed_ms = (end_2 - start_2) * 1000
    print(f"输出耗时: {elapsed_ms:.3f} 毫秒")
    del result
    del img
    gc.collect()
    elapsed_ms = (end_1 - start_1) * 1000
    print(f"操作耗时: {elapsed_ms:.3f} 毫秒")

    img2 = cv2.imread("garbage.bmp");
    result = cv2.convertScaleAbs(img2,alpha = 1.2,beta = 10);
    cv2.imwrite("garbage_out.bmp",result)
# 洗刷缓存区，防止连续测量时，缓存命中造成的加速
if __name__ == "__main__":
    main()