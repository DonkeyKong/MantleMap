#pragma once

#include <vector>
#include <string>

class ImageRGBA
{
public:
    ImageRGBA(std::string imagePath);
    uint8_t* data();
    int width();
    int height();
private:
    void read_png_file(const char* file_name);
    int width_;
    int height_;
    std::vector<uint8_t> data_;
};