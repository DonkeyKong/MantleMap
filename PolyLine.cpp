#include "PolyLine.hpp"
#include <iostream>
#include <math.h>

#define M_HALFPI 1.57079632679f

GfxProgram PolyLine::_program;
std::string PolyLine::_vertShaderName = "flatvertshader.glsl"; 
std::string PolyLine::_fragShaderName = "flatfragshader.glsl";
GLint PolyLine::_vertexAttrib;
GLint PolyLine::_colorAttrib;

PolyLine::PolyLine(MapState& map) : _map(map)
{    
    _dirty = true;
    _halfWidth = 0.5f;
    _locX = 0.0f;
    _locY = 0.0f;
    _color = {1.0f,1.0f,1.0f,1.0f};
}

void PolyLine::InitGL(MapState& map)
{  
  if (!_program.isLoaded)
  {
    // Load and compile the shaders into a glsl program
    _program.LoadShaders(map.GetResourcePath(_vertShaderName).c_str(),
                         map.GetResourcePath(_fragShaderName).c_str());
                         
    
    _vertexAttrib = glGetAttribLocation(_program.GetId(), "aVertex");
    _colorAttrib = glGetAttribLocation(_program.GetId(), "aColor");
  }
}

PolyLine::~PolyLine()
{
}

void PolyLine::SetPoints(const std::vector<PolyLinePoint>& points)
{
  _points = points;
  invalidateBuffers();
}

void PolyLine::AddPoint(const PolyLinePoint& point)
{
  _points.push_back(point);
  invalidateBuffers();
}

void PolyLine::SetLocation(float x, float y)
{
  _locX = x;
  _locY = y;
}

void PolyLine::Move(float dx, float dy)
{
  _locX += dx;
  _locY += dy;
}


void PolyLine::SetColor(Color color)
{
  _color = color;
}

void PolyLine::SetThickness(float thickness)
{
  _halfWidth = thickness / 2.0f;
  invalidateBuffers();
}

void PolyLine::invalidateBuffers()
{
  _dirty = true;
}

void PolyLine::createPoint(const PolyLinePoint& center, const PolyLinePoint& start, const PolyLinePoint& end, 
                 float angleDelta, PolyLinePoint& point)
{
  point = center;
  float baseAngle = atan2( end.y-start.y, end.x-start.x);
  point.x = center.x + _halfWidth * cos(baseAngle + angleDelta);
  point.y = center.y + _halfWidth * sin(baseAngle + angleDelta);
}

void PolyLine::updateBuffers()
{
  if (_dirty)
  {
    int numPts = _points.size();
    int numVerts = (numPts-1) * 4;
    
    if (numVerts <= 0)
    {
      _mesh.resize(0);
      _dirty = false;
      return;
    }
  
    // Regardless of what's about to happen, make sure the buffers are big enough
    if ((int)_mesh.size() != numVerts)
    {
      _mesh.resize(numVerts);
    }
    
    if (numPts > 1)
    {
      int vertIdx = 0;
      
      createPoint(_points[0], _points[0], _points[1], M_HALFPI, _mesh[vertIdx++]);
      createPoint(_points[0], _points[0], _points[1], -M_HALFPI, _mesh[vertIdx++]);
      
      for (int i=1; i < numPts-1; i++)
      {
        createPoint(_points[i], _points[i-1], _points[i], M_HALFPI, _mesh[vertIdx++]);
        createPoint(_points[i], _points[i-1], _points[i], -M_HALFPI, _mesh[vertIdx++]);
        createPoint(_points[i], _points[i], _points[i+1], M_HALFPI, _mesh[vertIdx++]);
        createPoint(_points[i], _points[i], _points[i+1], -M_HALFPI, _mesh[vertIdx++]);
      }
      
      createPoint(_points[numPts-1], _points[numPts-2], _points[numPts-1], M_HALFPI, _mesh[vertIdx++]);
      createPoint(_points[numPts-1], _points[numPts-2], _points[numPts-1], -M_HALFPI, _mesh[vertIdx++]);
    
    }
  
    _dirty = false;
  }
}

void PolyLine::Draw()
{
  std::lock_guard<std::mutex> lock(_mutex);

  updateBuffers();
  
  if (_mesh.size() >= 3)
  {
    // Select our shader program
    glUseProgram(_program.GetId());
  
    // Setup the map transform
    _program.SetCameraFromPixelTransform(_map.width,_map.height);
    _program.SetUniform("uLocation", _locX, _locY);
    _program.SetUniform("uColor", _color.r, _color.g, _color.b, _color.a);
    
    glVertexAttribPointer(
                        _vertexAttrib,      // The attribute ID
                        3,                  // size
                        GL_FLOAT,           // type
                        GL_FALSE,           // normalized?
                        7*4,                  // stride
                        (float*)&_mesh[0]         // underlying data
                );

     glEnableVertexAttribArray ( _vertexAttrib );
   
     glVertexAttribPointer(
                      _colorAttrib, // The attribute ID
                      4,                  // size
                      GL_FLOAT,           // type
                      GL_FALSE,           // normalized?
                      7*4,                  // stride
                      ((float*)&_mesh[0]+3)      // underlying data
              );
            
    glEnableVertexAttribArray(_colorAttrib);

    // Draw the triangles!
    glDrawArrays(GL_TRIANGLE_STRIP, 0, _mesh.size());
  }
}
