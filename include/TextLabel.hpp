#ifndef TEXTLABEL_HPP
#define TEXTLABEL_HPP

#include "SceneElement.hpp"
#include "Attributes.hpp"

#include <vector>
#include <string>
#include <mutex>

class TextLabel : public SceneElement
{
 public:
    TextLabel();
    virtual ~TextLabel();
    
    void SetText(std::string);
    void SetFontStyle(FontStyle style, float scale = 1.0f);
    void SetColor(float r, float g, float b, float a);
    void SetPosition(float x, float y);
    void SetFlowDirection(FlowDirection direction);
    void SetAlignment(HAlign alignment);
    float GetLength();

protected:
    virtual void initGL() override;
    virtual void drawInternal() override;

    void updateBuffers();
    void invalidateBuffers();
    float getFontTileWidth();
    float getFontTileHeight();
    GfxTexture& getFontTexture();
    
private:
    std::mutex _mutex;
    static std::unique_ptr<GfxProgram> _program;
    static std::unique_ptr<GfxTexture> _fontTextureLarge;
    static std::unique_ptr<GfxTexture> _fontTextureSmall;
    static std::unique_ptr<GfxTexture> _fontTextureBigTall;
    
    // Buffers containing render data
    bool _textDirty;
    FontStyle _fontStyle;
    HAlign _alignment;
    FlowDirection _direction;
    float _scale;
    std::string _text;
    Position2D _pos;
    Color _color;
    std::vector<float> _vertexXYZ;
    std::vector<TexCoord> _vertexUV;
};

#endif
