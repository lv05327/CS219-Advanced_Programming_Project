cmake_minimum_required(VERSION 3.10)
project(MyImageTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 引入 project_4 中的头文件
include_directories(../project_4/MyImageLib/include)

# 链接动态库所在目录
link_directories(../project_4/MyImageLib/build)

# 创建测试程序
add_executable(myimage myimage.cpp)

# 链接你的图像处理动态库
target_link_libraries(myimage MyImageLib)

# 链接 gperftools 的 tcmalloc 库
find_library(TCMALLOC_LIB tcmalloc)
if (TCMALLOC_LIB)
    message(STATUS "Found tcmalloc: ${TCMALLOC_LIB}")
    target_link_libraries(myimage ${TCMALLOC_LIB})
else()
    message(FATAL_ERROR "tcmalloc not found, please install gperftools via brew")
endif()

# OpenMP 设置（针对 AppleClang + Homebrew libomp）
target_compile_options(myimage PUBLIC -Xpreprocessor -fopenmp -I/opt/homebrew/opt/libomp/include)
target_link_libraries(myimage /opt/homebrew/opt/libomp/lib/libomp.dylib)
