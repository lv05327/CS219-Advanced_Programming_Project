cmake_minimum_required(VERSION 3.10)
project(MyImageLib)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

file(GLOB SOURCES "src/*.cpp")

add_library(MyImageLib SHARED ${SOURCES})

# ✅ OpenMP 设置（注意参数分开写）
set(OpenMP_CXX_FLAGS -Xpreprocessor -fopenmp -I/opt/homebrew/opt/libomp/include)
set(OpenMP_omp_LIBRARY /opt/homebrew/opt/libomp/lib/libomp.dylib)

# 应用编译和链接设置
target_compile_options(MyImageLib PUBLIC ${OpenMP_CXX_FLAGS})
target_link_libraries(MyImageLib PUBLIC ${OpenMP_omp_LIBRARY})

add_compile_options(-O3 -march=native -ftree-vectorize)