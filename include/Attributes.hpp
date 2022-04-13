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
  Vec3 operator-=(const Vec3& o)
  {
    return 
    {
      x-=o.x,
      y-=o.y,
      z-=o.z
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

  float mag2()
  {
    return pow(x,2.0f) +  pow(y,2.0f);
  }
  float mag()
  {
    return sqrt(pow(x,2.0f) +  pow(y,2.0f));
  }
  Vec2 operator-(const Vec2& o) const
  {
    return 
    {
      x-o.x,
      y-o.y
    };
  }
  Vec2 operator+(const Vec2& o) const
  {
    return 
    {
      x+o.x,
      y+o.y
    };
  }
  Vec2 operator+=(const Vec2& o)
  {
    return 
    {
      x+=o.x,
      y+=o.y
    };
  }
  Vec2 operator-=(const Vec2& o)
  {
    return 
    {
      x-=o.x,
      y-=o.y
    };
  }
  Vec2 operator*(const float& s) const
  {
    return 
    {
      x*s,
      y*s
    };
  }
  Vec2 operator/(const float& s) const
  {
    return 
    {
      x/s,
      y/s
    };
  }
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

struct Vec4
{
  float x{0}; 
  float y{0}; 
  float z{0};
  float w{0};
  float mag2()
  {
    return pow(x,2.0f) +  pow(y,2.0f) + pow(z,2.0f) + pow(w,2.0f);
  }
  float mag()
  {
    return sqrt(pow(x,2.0f) +  pow(y,2.0f) + pow(z,2.0f) + pow(w,2.0f));
  }
  float dot(const Vec4& o) const
  {
      return x*o.x + y*o.y + z*o.z + w*o.w;
  }
  Vec4 operator-(const Vec4& o) const
  {
    return 
    {
      x-o.x,
      y-o.y,
      z-o.z,
      w-o.w,
    };
  }
  Vec4 operator+(const Vec4& o) const
  {
    return 
    {
      x+o.x,
      y+o.y,
      z+o.z,
      w+o.w,
    };
  }
  Vec4 operator+=(const Vec4& o)
  {
    return 
    {
      x+=o.x,
      y+=o.y,
      z+=o.z,
      w+=o.w,
    };
  }
  Vec4 operator-=(const Vec4& o)
  {
    return 
    {
      x-=o.x,
      y-=o.y,
      z-=o.z,
      w-=o.w,
    };
  }
  Vec4 operator*(const float& s) const
  {
    return 
    {
      x*s,
      y*s,
      z*s,
      w*s,
    };
  }
  Vec4 operator/(const float& s) const
  {
    return 
    {
      x/s,
      y/s,
      z/s,
      w/s,
    };
  }
};

class Transform3D
{
private:
    Vec4 col0_{1, 0, 0, 0};
    Vec4 col1_{0, 1, 0, 0};
    Vec4 col2_{0, 0, 1, 0};
    Vec4 col3_{0, 0, 0, 1};
public:
    Vec4 row0() const { return {col0_.x, col1_.x, col2_.x, col3_.x}; }
    Vec4 row1() const { return {col0_.y, col1_.y, col2_.y, col3_.y}; }
    Vec4 row2() const { return {col0_.z, col1_.z, col2_.z, col3_.z}; }
    Vec4 row3() const { return {col0_.w, col1_.w, col2_.w, col3_.w}; }
    Vec4 col0() const { return col0_; }
    Vec4 col1() const { return col1_; }
    Vec4 col2() const { return col2_; }
    Vec4 col3() const { return col3_; }

    Transform3D operator*(const Transform3D& o) const
    {
        Transform3D result;
        result.col0_.x = row0().dot(o.col0());
        result.col0_.y = row1().dot(o.col0());
        result.col0_.z = row2().dot(o.col0());
        result.col0_.w = row3().dot(o.col0());
        result.col1_.x = row0().dot(o.col1());
        result.col1_.y = row1().dot(o.col1());
        result.col1_.z = row2().dot(o.col1());
        result.col1_.w = row3().dot(o.col1());
        result.col2_.x = row0().dot(o.col2());
        result.col2_.y = row1().dot(o.col2());
        result.col2_.z = row2().dot(o.col2());
        result.col2_.w = row3().dot(o.col2());
        result.col3_.x = row0().dot(o.col3());
        result.col3_.y = row1().dot(o.col3());
        result.col3_.z = row2().dot(o.col3());
        result.col3_.w = row3().dot(o.col3());
        return result;
    }

    void operator*=(const Transform3D& o)
    {
        (*this) = operator*(o);
    }

    const float* data() const
    {
        return (float*)(this);
    }

    static Transform3D FromEuler(float rX, float rY, float rZ)
    {
        Transform3D trX;
        trX.col1_.y = cos(rX);
        trX.col1_.z = sin(rX);
        trX.col2_.y = -sin(rX);
        trX.col2_.z = sin(rX);

        Transform3D trY;
        trY.col0_.x = cos(rY);
        trY.col0_.z = -sin(rY);
        trY.col2_.x = sin(rY);
        trY.col2_.z = cos(rY);

        Transform3D trZ;
        trZ.col0_.x = cos(rZ);
        trZ.col0_.y = sin(rZ);
        trZ.col1_.x = -sin(rZ);
        trZ.col1_.y = cos(rZ);

        return trZ * trY * trX;
    }

    static Transform3D FromTranslationAndScale(float tX, float tY, float tZ, float s)
    {
        Transform3D t;
        t.col0_.x = s;
        t.col1_.y = s;
        t.col2_.z = s;
        t.col3_.x = tX;
        t.col3_.y = tY;
        t.col3_.z = tZ;
        return t;
    }

    static Transform3D FromTranslationAndScale(float tX, float tY, float tZ, float sX, float sY, float sZ)
    {
        Transform3D t;
        t.col0_.x = sX;
        t.col1_.y = sY;
        t.col2_.z = sZ;
        t.col3_.x = tX;
        t.col3_.y = tY;
        t.col3_.z = tZ;
        return t;
    }

    static Transform3D FromTranslation(float tX, float tY, float tZ)
    {
        Transform3D t;
        t.col3_.x = tX;
        t.col3_.y = tY;
        t.col3_.z = tZ;
        return t;
    }

    static Transform3D FromScale(float sX, float sY, float sZ)
    {
        Transform3D t;
        t.col0_.x = sX;
        t.col1_.y = sY;
        t.col2_.z = sZ;
        return t;
    }

    static Transform3D FromScale(float s)
    {
        Transform3D t;
        t.col0_.x = s;
        t.col1_.y = s;
        t.col2_.z = s;
        return t;
    }
};

#pragma pack(pop)