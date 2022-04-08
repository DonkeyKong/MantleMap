#pragma once

#include <math.h>

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
struct Vec3
{
  float x{0}; 
  float y{0}; 
  float z{0};
  float mag2()
  {
    return pow(x,2.0f) +  pow(y,2.0f) + pow(z,2.0f);
  }
  float mag()
  {
    return sqrt(pow(x,2.0f) +  pow(y,2.0f) + pow(z,2.0f));
  }
  Vec3 operator-(const Vec3& o) const
  {
    return 
    {
      x-o.x,
      y-o.y,
      z-o.z
    };
  }
  Vec3 operator+(const Vec3& o) const
  {
    return 
    {
      x+o.x,
      y+o.y,
      z+o.z
    };
  }
  Vec3 operator+=(const Vec3& o)
  {
    return 
    {
      x+=o.x,
      y+=o.y,
      z+=o.z
    };
  }
  Vec3 operator*(const float& s) const
  {
    return 
    {
      x*s,
      y*s,
      z*s
    };
  }
  Vec3 operator/(const float& s) const
  {
    return 
    {
      x/s,
      y/s,
      z/s
    };
  }
};

struct Vec2
{
  float x{0}; 
  float y{0};
};

typedef Vec3 Position;
typedef Vec2 Position2D;

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
  Vec3 velocity{0,0,0};
};

#pragma pack(pop)