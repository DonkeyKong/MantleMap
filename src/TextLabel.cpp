#include "TextLabel.hpp"

#include "GLError.hpp"

std::unique_ptr<GfxProgram> TextLabel::_program;
std::unique_ptr<GfxTexture> TextLabel::_fontTextureLarge;
std::unique_ptr<GfxTexture> TextLabel::_fontTextureSmall;
std::unique_ptr<GfxTexture> TextLabel::_fontTextureBigTall;

TextLabel::TextLabel(ConfigService& map) : SceneElement(map)
{
  _scale = 1.0f;
  _alignment = HAlign::Left;
  _direction = FlowDirection::Horizontal;
  _fontStyle = FontStyle::Regular;
  _textDirty = true;
}

void TextLabel::initGL()
{  
  if (!_program)
  {
    // Load and compile the shaders into a glsl program
    _program = loadProgram("vertshader.glsl", "fragshader.glsl", 
    {
        ShaderFeature::PixelSnap,
        ShaderFeature::Texture
    });
    
    _fontTextureLarge = loadTexture("font_6x6.png");
    _fontTextureSmall = loadTexture("font_4x6.png");
    _fontTextureBigTall = loadTexture("font_8x12.png");
    print_if_glerror("InitGL for TextLabel");
  }
}

TextLabel::~TextLabel()
{
}

void TextLabel::SetText(std::string text)
{
  if (text != _text)
  {
    std::lock_guard<std::mutex> lock(_mutex);
    _text = text;
    _textDirty = true;
  }
}

void TextLabel::SetFlowDirection(FlowDirection direction)
{
  if (_direction != direction)
  {
    std::lock_guard<std::mutex> lock(_mutex);
    _direction = direction;
    invalidateBuffers();
  }
}

void TextLabel::SetFontStyle(FontStyle fontStyle, float scale)
{
  if (_fontStyle != fontStyle || _scale != scale)
  {
    std::lock_guard<std::mutex> lock(_mutex);
    _fontStyle = fontStyle;
    _scale = scale;
  }
}

void TextLabel::SetAlignment(HAlign alignment)
{
  if (_alignment != alignment)
  {
    std::lock_guard<std::mutex> lock(_mutex);
    _alignment = alignment;
  }
}

void TextLabel::SetColor(float r, float g, float b, float a)
{
  _color = {r, g, b, a};
}

void TextLabel::SetPosition(float x, float y)
{
  _pos = {x, y};
}

void TextLabel::invalidateBuffers()
{
  // Clear the vertex buffer
  _vertexXYZ.resize(0);
}

float TextLabel::getFontTileWidth()
{
  if (_fontStyle == FontStyle::Regular)
    return 6.0f;
  else if (_fontStyle == FontStyle::Narrow)
    return 4.0f;
  else if (_fontStyle == FontStyle::BigNarrow)
    return 8.0f;
    
  return 16.0f;
}

float TextLabel::getFontTileHeight()
{
  if (_fontStyle == FontStyle::Regular)
    return 6.0f;
  else if (_fontStyle == FontStyle::Narrow)
    return 6.0f;
  else if (_fontStyle == FontStyle::BigNarrow)
    return 12.0f;
    
  return 16.0f;
}

GfxTexture& TextLabel::getFontTexture()
{
  if (_fontStyle == FontStyle::Regular)
    return *_fontTextureLarge;
  else if (_fontStyle == FontStyle::Narrow)
    return *_fontTextureSmall;
  else if (_fontStyle == FontStyle::BigNarrow)
    return *_fontTextureBigTall;
    
  return *_fontTextureLarge;
}

void TextLabel::updateBuffers()
{
  // Regardless of what's about to happen, make sure the geometry buffers are big enough
  if (_vertexXYZ.size() < _text.size() * 18)
  {
    int oldSize = _vertexXYZ.size() / 18;
    _vertexXYZ.resize(_text.size() * 18);
        
    int dx = _direction==FlowDirection::Horizontal ? 1 : 0;
    int dy = _direction==FlowDirection::Vertical ? 1 : 0;
    
    // Setup the triangle vertices
    //  0--2      3
    //  | /     / |
    //  1      4--5
    
    for (int i = oldSize; i < (int)_text.size(); i++)
    {
      _vertexXYZ[i*18+0 ] = 0+i*dx; _vertexXYZ[i*18+1 ] = 0+i*dy; _vertexXYZ[i*18+2 ] = 0;
      _vertexXYZ[i*18+3 ] = 0+i*dx; _vertexXYZ[i*18+4 ] = 1+i*dy; _vertexXYZ[i*18+5 ] = 0;
      _vertexXYZ[i*18+6 ] = 1+i*dx; _vertexXYZ[i*18+7 ] = 0+i*dy; _vertexXYZ[i*18+8 ] = 0;
      
      _vertexXYZ[i*18+9 ] = 1+i*dx; _vertexXYZ[i*18+10] = 0+i*dy; _vertexXYZ[i*18+11] = 0;
      _vertexXYZ[i*18+12] = 0+i*dx; _vertexXYZ[i*18+13] = 1+i*dy; _vertexXYZ[i*18+14] = 0;
      _vertexXYZ[i*18+15] = 1+i*dx; _vertexXYZ[i*18+16] = 1+i*dy; _vertexXYZ[i*18+17] = 0;
    }
  }

  if (_textDirty)
  {
    if (_vertexUV.size() != _text.size() * 6)
      _vertexUV.resize(_text.size() * 6);
    
    float ldx = 1.0/16.0; // The font textures are 16x16
    float ldy = 1.0/16.0; // The font textures are 16x16
    for (int i = 0; i < (int)_text.size(); i++)
    {
      // Setup the texture coords
      float lX = (float)(_text[i] % 16) * ldx;
      float lY = (float)(_text[i] / 16 + 1) * ldy;
      _vertexUV[i*6+0] = { lX,       lY - ldy };
      _vertexUV[i*6+1] = { lX,       lY       }; 
      _vertexUV[i*6+2] = { lX + ldx, lY - ldy }; 
      _vertexUV[i*6+3] = { lX + ldx, lY - ldy };
      _vertexUV[i*6+4] = { lX,       lY       }; 
      _vertexUV[i*6+5] = { lX + ldx, lY       }; 
    }
  
    _textDirty = false;
  }
}

float TextLabel::GetLength()
{
  if (_direction==FlowDirection::Horizontal)
    return _text.size() * getFontTileWidth() * _scale;
  else
    return _text.size() * getFontTileHeight() * _scale;
}

void TextLabel::drawInternal()
{
  std::lock_guard<std::mutex> lock(_mutex);

  updateBuffers();

  // Don't draw anything if there is no text
  if (_text.size() == 0)
    return;
  
	// Select our shader program
    _program->Use();
	
    // Bind the font texture to texture unit 0
    _program->SetTexture0(getFontTexture());
  
    // Set the color of the text
    _program->SetTint( _color);
    
    Position2D pos = _pos;

    // Adjust position given our flow and alignment
    if (_alignment == HAlign::Right && _direction==FlowDirection::Horizontal)
    {
        pos -= {GetLength(), 0};
    }
    else if (_alignment == HAlign::Right && _direction==FlowDirection::Vertical)
    {
        pos -= {0, GetLength()};
    }
    else if (_alignment == HAlign::Center && _direction==FlowDirection::Horizontal)
    {
        pos -= {GetLength() / 2.0f, 0};
    }
    else if (_alignment == HAlign::Center && _direction==FlowDirection::Vertical)
    {
        pos -= {0, GetLength() / 2.0f};
    }
    
    _program->SetModelTransform(pos.x, pos.y, getFontTileWidth() * _scale, getFontTileHeight() * _scale);
        
    glVertexAttribPointer(
                        _program->Attrib("aPosition"),      // The attribute ID
                        3,                  // size
                        GL_FLOAT,           // type
                        GL_FALSE,           // normalized?
                        0,                  // stride
                        &_vertexXYZ[0]         // underlying data
                );

    glEnableVertexAttribArray (_program->Attrib("aPosition") );
    
    glVertexAttribPointer(
                        _program->Attrib("aTexCoord"), // The attribute ID
                        2,                  // size
                        GL_FLOAT,           // type
                        GL_FALSE,           // normalized?
                        0,                  // stride
                        &_vertexUV[0]      // underlying data
                );
                
    glEnableVertexAttribArray(_program->Attrib("aTexCoord"));

    // Draw the triangles!
    glDrawArrays(GL_TRIANGLES, 0, _text.size() * 2 * 3);

    print_if_glerror("Internal draw for TextLabel");
}
