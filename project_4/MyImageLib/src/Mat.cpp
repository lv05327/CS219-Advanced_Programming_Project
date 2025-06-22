#include <Mat.h>
#include <cstring>
#include <stdexcept>

Mat::Mat(int width, int height, int channels)
    : width(width), height(height), channels(channels),
      data(new unsigned char[width * height * channels]) {}

Mat::Mat(const Mat& other)
    : width(other.width), height(other.height), channels(other.channels), data(other.data) {}

Mat& Mat::operator=(const Mat& other) {
    if (this != &other) {
        width = other.width;
        height = other.height;
        channels = other.channels;
        data = other.data;
    }
    return *this;
}

int Mat::getWidth() const { return width; }
int Mat::getHeight() const { return height; }
int Mat::getChannels() const { return channels; }

unsigned char* Mat::getWritableData() { return data.get(); }
const unsigned char* Mat::getData() const { return data.get(); }

Mat Mat::clone() const {
    Mat copy(width, height, channels);
    std::memcpy(copy.data.get(), data.get(), width * height * channels);
    return copy;
}

