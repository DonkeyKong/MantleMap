#pragma once

#include "ImageRGBA.hpp"
#include "GLES2/gl2.h"

class GfxTexture
{
public:
    GfxTexture(int width, int height);
    GfxTexture(const ImageRGBA& image);
    GfxTexture(const std::string& imagePath);
    void LoadImageToTexture(const ImageRGBA& image);
    ~GfxTexture();
    GLuint GetId() const;
    int GetWidth() const;
    int GetHeight() const;
private:
    GLuint textureID{0};
    int height{0};
    int width{0};
};