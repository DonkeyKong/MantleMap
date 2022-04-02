#pragma once

struct HSVColor;
struct Color;

enum class HAlign
{
    Left, Center, Right
};

enum class VAlign
{
    Top, Center, Bottom
};

enum class FlowDirection
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

#pragma pack(push, 1)
struct Position
{
  float x{0}; 
  float y{0}; 
  float z{0};
};

struct Position2D
{
  float x{0}; 
  float y{0};
};

struct TexCoord
{
  float u{0}; 
  float v{0};
};

struct Color
{
  float r{1};
  float g{1};
  float b{1};
  float a{1};
  Color() = default;
  Color(float r, float g, float b, float a);
  Color(const HSVColor& hsv);
};

struct HSVColor
{
  float h{1};
  float s{1};
  float v{1};
  float a{1};
  HSVColor() = default;
  HSVColor(float h, float s, float v, float a);
  HSVColor(const Color& rgb);
};

struct Vertex
{
  Position pos{0,0,0};
  Color color{1,1,1,1};
};

struct PhysicsPoint
{
  Position pos{0,0,0};
  Color color{1,1,1,1};
  float size{1};
  float mass{1};
  Position velocity{0,0,0};
};

#pragma pack(pop)