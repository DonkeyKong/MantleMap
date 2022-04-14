#pragma once

#include <vector>
#include <string>
#include <mutex>

#include "GfxProgram.hpp"
#include "SceneElement.hpp"
#include "ConfigService.hpp"
#include "Attributes.hpp"

enum class MeshMode : GLenum
{
    Fan = GL_TRIANGLE_FAN,
    Strip = GL_TRIANGLE_STRIP,
    Triangles = GL_TRIANGLES
};

class PolyFill : public SceneElement
{
 public:
    PolyFill(ConfigService&);
    virtual ~PolyFill();
    void SetMeshMode();
    void SetPoints(const std::vector<Vertex>& points);
    void AddPoint(const Vertex& point);
    void SetLocation(float x, float y);
    void Move(float dx, float dy);
    void SetColor(Color c);
protected:
    virtual void initGL() override;
    virtual void drawInternal() override;
    
    void updateBuffers();
    void invalidateBuffers();
    
private:
    std::mutex _mutex;
    static std::unique_ptr<GfxProgram> _program;
    
    // Buffers containing render data
    bool _dirty;
    Position _loc;
    Color _color;
    std::vector<Vertex> _points;
    std::vector<Vertex> _mesh;
    MeshMode _meshMode;
};

