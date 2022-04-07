#pragma once

#include "LoadShaders.hpp"
#include "SceneElement.hpp"
#include "ConfigService.hpp"

class ImageView : public SceneElement
{
 public:
    ImageView(ConfigService&);
    virtual ~ImageView();
    void SetImage(ImageRGBA image);
    void SetPosition(float x, float y);
    void SetColor(float r, float g, float b, float a);
    void SetScale(float scale);
    int GetWidth();
    int GetHeight();
protected:
    virtual void initGL() override;
    virtual void drawInternal() override;
private:
    float x, y, r, g, b, a, scale;
    std::vector<float> mesh;
    GLuint texture;
    ImageRGBA image;
    bool dirty;
    static GfxProgram _program;
    static GLint _vertexAttrib;
    static GLint _textureAttrib;
};

