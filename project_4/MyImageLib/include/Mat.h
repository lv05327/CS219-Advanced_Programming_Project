#ifndef MAT_H
#define MAT_H

#include <string>
#include <memory>

class Mat {
public:
    Mat(int width, int height, int channels);
    Mat(const Mat& other);              // 浅拷贝构造
    Mat& operator=(const Mat& other);   // 浅拷贝赋值
    ~Mat() = default;                   // 智能指针自动释放

    Mat adjustBrightnessContrast(int value,float alpha)const;
    friend Mat blend(const Mat& a, const Mat& b);
    Mat resize(int new_width, int new_height) const;
    void size_of()const;
    Mat flip() const;
    Mat toGrayscale() const;
    Mat crop(int x, int y, int new_width, int new_height) const;
    Mat clone() const;                // 深拷贝

    int getWidth() const;
    int getHeight() const;
    int getChannels() const;
    unsigned char* getWritableData();              // 可修改数据访问
    const unsigned char* getData() const;          // 只读数据访问

private:
    int width, height, channels;
    std::shared_ptr<unsigned char[]> data;   
};

//接口
Mat imread(const std::string& filename);
bool imwrite(const std::string& filename, const Mat& mat);
void imshow(const Mat& img);
void help();
#endif
