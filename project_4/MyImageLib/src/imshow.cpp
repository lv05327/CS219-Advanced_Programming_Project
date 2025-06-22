#include "Mat.h"
#include <cstdlib>
#include <string>
#include <iostream>
#include <chrono>
#include <thread>
#include <cstdio>

#ifdef _WIN32
    #include <windows.h>
    #define OS_WINDOWS
#elif defined(__APPLE__)
    #include <unistd.h>
    #define OS_MAC
#elif defined(__linux__)
    #include <unistd.h>
    #define OS_LINUX
#endif

void imshow(const Mat& img) {
    std::string tmpFile = "__temp_imshow__.bmp";
    if (!imwrite(tmpFile, img)) {
        std::cerr << "Failed to write image to disk.\n";
        return;
    }

#ifdef OS_WINDOWS
    std::string command = "start \"\" \"" + tmpFile + "\"";
#elif defined(OS_MAC)
    std::string command = "open \"" + tmpFile + "\"";
#elif defined(OS_LINUX)
    std::string command = "xdg-open \"" + tmpFile + "\"";
#else
    std::cerr << "Unsupported OS.\n";
    return;
#endif

    int result = std::system(command.c_str());
    if (result != 0) {
        std::cerr << "Failed to open image.\n";
        return;
    }

    std::cout << "Image shown. Press ENTER to close and delete temp file...\n";
    std::cin.get();

    if (std::remove(tmpFile.c_str()) != 0) {
        std::cerr << "Warning: failed to delete temp file.\n";
    }
}
