#pragma once

#include <vector>
#include <string>
#include <memory>

class ImageRGBA
{
public:
    ImageRGBA();
    ImageRGBA(int width, int height);
    uint8_t* data();
    const uint8_t* data() const;
    int width() const;
    int height() const;
    int padW() const;
    int padH() const;
    void PadToPowerOfTwo();
    static std::shared_ptr<ImageRGBA> FromPngFile(const std::string& imagePath);
    static std::shared_ptr<ImageRGBA> FromQrPayload(const std::string& qrPayload);
    uint8_t& operator[](std::size_t idx);
private:
    void read_png_file(const char* file_name);
    int width_, height_, padW_, padH_;
    std::vector<uint8_t> data_;
};