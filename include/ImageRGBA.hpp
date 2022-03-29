#pragma once

#include <vector>
#include <string>

class ImageRGBA
{
public:
    ImageRGBA();
    ImageRGBA(int width, int height);
    uint8_t* data();
    int width();
    int height();
    int padW();
    int padH();
    void PadToPowerOfTwo();
    static ImageRGBA FromPngFile(std::string imagePath);
    static ImageRGBA FromQrPayload(std::string qrPayload);
    uint8_t& operator[](std::size_t idx);
private:
    void read_png_file(const char* file_name);
    int width_, height_, padW_, padH_;
    std::vector<uint8_t> data_;
};