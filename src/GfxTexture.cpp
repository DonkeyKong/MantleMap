#include "GfxTexture.hpp"

// Some day we might need to compile without NPOT. 
// Here's some reference info to help with that!
// #ifdef NPOT_TEXTURE_SUPPORT
// float maxU = 1.0f;
// float maxV = 1.0f;
// #else
// // Something subtle is wrong here causing the texture mapping to be off by a pixel or two
// image.PadToPowerOfTwo();
// float maxU = 1.0f - ((float)image.padW() / (float)image.width());
// float maxV = 1.0f - ((float)image.padH() / (float)image.height());
// #endif

GfxTexture::GfxTexture(int width, int height)
{
    #ifndef NPOT_TEXTURE_SUPPORT
    throw std::runtime_error("Non-power-of-two textures are unsupported. Fix the NPOT code!");
    #endif
    this->width = width;
    this->height = height;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0 , GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

GfxTexture::GfxTexture(const ImageRGBA& image)
{
    #ifndef NPOT_TEXTURE_SUPPORT
    throw std::runtime_error("Non-power-of-two textures are unsupported. Fix the NPOT code!");
    #endif
    width = image.width();
    height = image.height();
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0 , GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

GfxTexture::GfxTexture(const std::string& imagePath)
{
    #ifndef NPOT_TEXTURE_SUPPORT
    throw std::runtime_error("Non-power-of-two textures are unsupported. Fix the NPOT code!");
    #endif
    auto image = ImageRGBA::FromPngFile(imagePath);
    width = image->width();
    height = image->height();
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0 , GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

void GfxTexture::LoadImageToTexture(const ImageRGBA& image)
{
    #ifndef NPOT_TEXTURE_SUPPORT
    throw std::runtime_error("Non-power-of-two textures are unsupported. Fix the NPOT code!");
    #endif
    width = image.width();
    height = image.height();
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0 , GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

GfxTexture::~GfxTexture()
{
    if (textureID != 0)
        glDeleteTextures(1, &textureID);
    textureID = 0;
}

GLuint GfxTexture::GetId() const 
{ 
    return textureID; 
}

int GfxTexture::GetWidth() const 
{ 
    return width; 
}

int GfxTexture::GetHeight() const 
{ 
    return height; 
}