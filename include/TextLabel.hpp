#ifndef TEXTLABEL_HPP
#define TEXTLABEL_HPP

#include "LoadShaders.hpp"
#include "MapState.hpp"

#include <vector>
#include <string>
#include <mutex>

enum class TextFlowDirection
{
  Horizontal,
  Vertical
};

enum class FontStyle
{
  Narrow,
  Regular,
  BigNarrow
};

enum class TextAlignment
{
  AlignToStart,
  AlignToEnd,
  Center
};

class TextLabel
{
 public:
    TextLabel(MapState&);
    ~TextLabel();
    
    static void InitGL(MapState& map);
    void SetText(std::string);
    void SetFontStyle(FontStyle style, float scale = 1.0f);
    void SetColor(float r, float g, float b, float a);
    void SetPosition(float x, float y);
    void SetFlowDirection(TextFlowDirection direction);
    void SetAlignment(TextAlignment alignment);
    void Draw();
    float GetLength();

protected:
    void updateBuffers();
    void invalidateBuffers();
    float getFontTileWidth();
    float getFontTileHeight();
    GLuint getFontTexture();
    
private:
    std::mutex _mutex;
    MapState& _map;
    static GfxProgram _program;
    static GLuint _fontTextureLarge;
    static GLuint _fontTextureSmall;
    static GLuint _fontTextureBigTall;
    static std::string _vertShaderName; 
    static std::string _fragShaderName;
    static GLint _vertexAttrib;
    static GLint _coordinateAttrib;
    
    // Buffers containing render data
    bool _textDirty;
    FontStyle _fontStyle;
    TextAlignment _alignment;
    TextFlowDirection _direction;
    float _scale;
    std::string _text;
    float _r, _g, _b, _a, _x, _y;
    std::vector<float> _vertex;
    std::vector<float> _texCoords;
    std::vector<float> _color;
    
};

#endif
