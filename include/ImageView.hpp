#pragma once

#include "Attributes.hpp"
#include "GfxTexture.hpp"
#include "GfxProgram.hpp"
#include "SceneElement.hpp"
#include "ConfigService.hpp"

class ImageView : public SceneElement
{
 public:
    ImageView();
    virtual ~ImageView();
    void SetImage(std::shared_ptr<ImageRGBA> image);
    void SetPosition(float x, float y);
    void SetColor(float r, float g, float b, float a);
    void SetScale(float scale);
    int GetWidth();
    int GetHeight();
protected:
    virtual void initGL() override;
    virtual void drawInternal() override;
private:
    float x, y, scale;
    Color tint;
    bool dirty;
    std::shared_ptr<ImageRGBA> image;
    std::vector<float> mesh;
    std::unique_ptr<GfxTexture> texture;
    static std::unique_ptr<GfxProgram> _program;
};

