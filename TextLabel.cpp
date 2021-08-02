#include "TextLabel.hpp"
#include <iostream>

GfxProgram TextLabel::_program;
std::string TextLabel::_vertShaderName = "fontvertshader.glsl"; 
std::string TextLabel::_fragShaderName = "fontfragshader.glsl";
GLuint TextLabel::_fontTextureLarge = 0;
GLuint TextLabel::_fontTextureSmall = 0;
GLuint TextLabel::_fontTextureBigTall = 0;
GLint TextLabel::_vertexAttrib;
GLint TextLabel::_coordinateAttrib;

TextLabel::TextLabel(MapState& map) : _map(map)
{
  _r = 1.0;
  _g = 1.0;
  _b = 1.0;
  _a = 1.0;
  _x = 0;
  _y = 0;
  _scale = 1.0f;
  _alignment = TextAlignment::AlignToStart;
  _direction = TextFlowDirection::Horizontal;
  _fontStyle = FontStyle::Regular;
  _textDirty = true;
}

void TextLabel::InitGL(MapState& map)
{  
  if (!_program.isLoaded)
  {
    // Load and compile the shaders into a glsl program
    _program.LoadShaders(map.GetResourcePath(_vertShaderName).c_str(),
                         map.GetResourcePath(_fragShaderName).c_str());
    
    _fontTextureLarge = LoadImageToTexture(map.GetResourcePath("font_6x6.png"));
    _fontTextureSmall = LoadImageToTexture(map.GetResourcePath("font_4x6.png"));
    _fontTextureBigTall = LoadImageToTexture(map.GetResourcePath("font_8x12.png"));
    
    _vertexAttrib = glGetAttribLocation(_program.GetId(), "aVertex");
    _coordinateAttrib = glGetAttribLocation(_program.GetId(), "aTexCoord");
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

void TextLabel::SetFlowDirection(TextFlowDirection direction)
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

void TextLabel::SetAlignment(TextAlignment alignment)
{
  if (_alignment != alignment)
  {
    std::lock_guard<std::mutex> lock(_mutex);
    _alignment = alignment;
  }
}

void TextLabel::SetColor(float r, float g, float b, float a)
{
  _r = r;
  _g = g;
  _b = b;
  _a = a;
}

void TextLabel::SetPosition(float x, float y)
{
  _x = x;
  _y = y;
}

void TextLabel::invalidateBuffers()
{
  // Clear the vertex buffer
  _vertex.resize(0);
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

GLuint TextLabel::getFontTexture()
{
  if (_fontStyle == FontStyle::Regular)
    return _fontTextureLarge;
  else if (_fontStyle == FontStyle::Narrow)
    return _fontTextureSmall;
  else if (_fontStyle == FontStyle::BigNarrow)
    return _fontTextureBigTall;
    
  return _fontTextureLarge;
}

void TextLabel::updateBuffers()
{
  // Regardless of what's about to happen, make sure the geometry buffers are big enough
  if (_vertex.size() < _text.size() * 18)
  {
    int oldSize = _vertex.size() / 18;
    _vertex.resize(_text.size() * 18);
    
    //std::cout << "Old size was " << oldSize << ". Expanding to " << _text.size() << std::endl;
    
    int dx = _direction==TextFlowDirection::Horizontal ? 1 : 0;
    int dy = _direction==TextFlowDirection::Vertical ? 1 : 0;
    
    // Setup the triangle vertices
    //  0--2      3
    //  | /     / |
    //  1      4--5
    
    for (int i = oldSize; i < (int)_text.size(); i++)
    {
      _vertex[i*18+0 ] = 0+i*dx; _vertex[i*18+1 ] = 0+i*dy; _vertex[i*18+2 ] = 0;
      _vertex[i*18+3 ] = 0+i*dx; _vertex[i*18+4 ] = 1+i*dy; _vertex[i*18+5 ] = 0;
      _vertex[i*18+6 ] = 1+i*dx; _vertex[i*18+7 ] = 0+i*dy; _vertex[i*18+8 ] = 0;
      
      _vertex[i*18+9 ] = 1+i*dx; _vertex[i*18+10] = 0+i*dy; _vertex[i*18+11] = 0;
      _vertex[i*18+12] = 0+i*dx; _vertex[i*18+13] = 1+i*dy; _vertex[i*18+14] = 0;
      _vertex[i*18+15] = 1+i*dx; _vertex[i*18+16] = 1+i*dy; _vertex[i*18+17] = 0;
    }
  }

  if (_textDirty)
  {
    if (_texCoords.size() != _text.size() * 6 * 2)
      _texCoords.resize(_text.size() * 6 * 2);
    
    float ldx = 1.0/16.0; // The font textures are 16x16
    float ldy = 1.0/16.0; // The font textures are 16x16
    for (int i = 0; i < (int)_text.size(); i++)
    {
      // Setup the texture coords
      float lX = (float)(_text[i] % 16) * ldx;
      float lY = (float)(_text[i] / 16 + 1) * ldy;
      _texCoords[i*12+0 ] = lX;       _texCoords[i*12+1 ] = lY - ldy;
      _texCoords[i*12+2 ] = lX;       _texCoords[i*12+3 ] = lY; 
      _texCoords[i*12+4 ] = lX + ldx; _texCoords[i*12+5 ] = lY - ldy; 
      
      _texCoords[i*12+6 ] = lX + ldx; _texCoords[i*12+7 ] = lY - ldy;
      _texCoords[i*12+8 ] = lX;       _texCoords[i*12+9 ] = lY; 
      _texCoords[i*12+10] = lX + ldx; _texCoords[i*12+11] = lY; 
    }
  
    _textDirty = false;
  }
}

float TextLabel::GetLength()
{
  if (_direction==TextFlowDirection::Horizontal)
    return _text.size() * getFontTileWidth() * _scale;
  else
    return _text.size() * getFontTileHeight() * _scale;
}

void TextLabel::Draw()
{
  std::lock_guard<std::mutex> lock(_mutex);

  updateBuffers();
  
	// Select our shader program
	glUseProgram(_program.GetId());
	
	// Setup the map transform
	_program.SetCameraFromPixelTransform(_map.width,_map.height);
	
	// Bind the day, night, and lon lat lookup textures to units 0, 1, and 2
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, getFontTexture());
  
  // Tell our shader which units to look for each texture on
  _program.SetUniform("uFontTexture", 0);
  
  // Set a couple more uniforms
  _program.SetUniform("uColor", _r, _g, _b, _a);
  
  if (_alignment == TextAlignment::AlignToStart)
  {
    _program.SetUniform("uLocation", _x, _y);
  }
  else if (_alignment == TextAlignment::AlignToEnd)
  {
    if (_direction==TextFlowDirection::Horizontal)
    {
      _program.SetUniform("uLocation", _x - GetLength(), _y);
    }
    else
    {
      _program.SetUniform("uLocation", _x, _y - GetLength());
    }
  }
  else
  {
    if (_direction==TextFlowDirection::Horizontal)
    {
      _program.SetUniform("uLocation", _x - GetLength() / 2.0f, _y);
    }
    else
    {
      _program.SetUniform("uLocation", _x, _y - GetLength() / 2.0f);
    }
  }
  
  _program.SetUniform("uFontSizePx", getFontTileWidth() * _scale, getFontTileHeight() * _scale);
    
  glVertexAttribPointer(
                      _vertexAttrib,      // The attribute ID
                      3,                  // size
                      GL_FLOAT,           // type
                      GL_FALSE,           // normalized?
                      0,                  // stride
                      &_vertex[0]         // underlying data
              );

   glEnableVertexAttribArray ( _vertexAttrib );
   
   glVertexAttribPointer(
                    _coordinateAttrib, // The attribute ID
                    2,                  // size
                    GL_FLOAT,           // type
                    GL_FALSE,           // normalized?
                    0,                  // stride
                    &_texCoords[0]      // underlying data
            );
            
  glEnableVertexAttribArray(_coordinateAttrib);

  // Draw the triangles!
  glDrawArrays(GL_TRIANGLES, 0, _text.size() * 2 * 3);
}
