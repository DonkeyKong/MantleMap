#include "PolyFill.hpp"
#include <iostream>
#include <math.h>

#define M_HALFPI 1.57079632679f

std::unique_ptr<GfxProgram> PolyFill::_program;

PolyFill::PolyFill()
{    
    _dirty = true;
    _meshMode = MeshMode::Fan;
    _loc = {0.0f, 0.0f, 0.0f};
    _color = {1.0f,1.0f,1.0f,1.0f};
}

void PolyFill::initGL()
{  
  if (!_program)
  {
    // Load and compile the shaders into a glsl program
    _program = loadProgram( "vertshader.glsl", "fragshader.glsl", 
                            {
                                ShaderFeature::VertexColor
                            });
  }
}

PolyFill::~PolyFill()
{
}

void PolyFill::SetPoints(const std::vector<Vertex>& points)
{
  _points = points;
  invalidateBuffers();
}

void PolyFill::AddPoint(const Vertex& point)
{
  _points.push_back(point);
  invalidateBuffers();
}

void PolyFill::SetLocation(float x, float y)
{
  _loc = {x,y,0};
}

void PolyFill::Move(float dx, float dy)
{
  _loc += {dx,dy,0};
}

void PolyFill::SetColor(Color color)
{
  _color = color;
}

void PolyFill::invalidateBuffers()
{
  _dirty = true;
}

void PolyFill::updateBuffers()
{
  if (_dirty)
  {
    _mesh = _points;
    _dirty = false;
  }
}

void PolyFill::drawInternal()
{
  std::lock_guard<std::mutex> lock(_mutex);

  updateBuffers();
  
  if (_mesh.size() >= 3)
  {
    // Select our shader program
    _program->Use();
  
    // Set the position and color
    _program->SetModelTransform(Transform3D::FromTranslation(_loc.x, _loc.y, 0));
    _program->SetTint(_color);
    
    glVertexAttribPointer(
                        _program->Attrib("aPosition"),      // The attribute ID
                        3,                  // size
                        GL_FLOAT,           // type
                        GL_FALSE,           // normalized?
                        sizeof(Vertex),                  // stride
                        (float*)&_mesh[0]         // underlying data
                );

     glEnableVertexAttribArray ( _program->Attrib("aPosition") );
   
     glVertexAttribPointer(
                      _program->Attrib("aColor"), // The attribute ID
                      4,                  // size
                      GL_FLOAT,           // type
                      GL_FALSE,           // normalized?
                      sizeof(Vertex),                  // stride
                      ((float*)&_mesh[0]+3)      // underlying data
              );
            
    glEnableVertexAttribArray(_program->Attrib("aColor"));

    // Draw the mesh!
    glDrawArrays(static_cast<GLenum>(_meshMode), 0, _mesh.size());
  }
}
