#ifndef PolyLine_HPP
#define PolyLine_HPP

#include "EGL/egl.h"
#include "EGL/eglplatform.h"
#include "GLES2/gl2.h"
#include "EGL/eglext.h"
#include <vector>
#include <string>
#include <mutex>

#include "LoadShaders.hpp"
#include "MapState.hpp"

struct PolyLinePoint
{
  float x, y, z, r, g, b, a;
};

struct Color
{
  float r, g, b, a;
};

class PolyLine
{
 public:
    PolyLine(MapState&);
    ~PolyLine();
    
    static void InitGL(MapState& map);
    void SetPoints(const std::vector<PolyLinePoint>& points);
    void AddPoint(const PolyLinePoint& point);
    void SetThickness(float thickness);
    void SetLocation(float x, float y);
    void Move(float dx, float dy);
    void SetColor(Color c);
    void Draw();

protected:
    void createPoint(const PolyLinePoint& center, const PolyLinePoint& start, const PolyLinePoint& end, 
                      float angleDelta, PolyLinePoint& point);
    void updateBuffers();
    void invalidateBuffers();
    
private:
    std::mutex _mutex;
    MapState& _map;
    static GfxProgram _program;
    static std::string _vertShaderName; 
    static std::string _fragShaderName;
    static GLint _vertexAttrib;
    static GLint _colorAttrib;
    
    // Buffers containing render data
    bool _dirty;
    float _halfWidth;
    float _locX, _locY;
    Color _color;
    std::vector<PolyLinePoint> _points;
    std::vector<PolyLinePoint> _mesh;
};

#endif
