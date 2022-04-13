#pragma once

#include <vector>
#include <string>
#include <mutex>

#include "GfxProgram.hpp"
#include "SceneElement.hpp"
#include "ConfigService.hpp"
#include "Attributes.hpp"

class PolyLine : public SceneElement
{
 public:
    PolyLine(ConfigService&);
    virtual ~PolyLine();

    void SetPoints(const std::vector<Vertex>& points);
    void AddPoint(const Vertex& point);
    void SetThickness(float thickness);
    void SetLocation(float x, float y);
    void Move(float dx, float dy);
    void SetColor(Color c);
protected:
    virtual void initGL() override;
    virtual void drawInternal() override;
    
    void createPoint(const Vertex& center, const Vertex& start, const Vertex& end, 
                      float angleDelta, Vertex& point);
    void updateBuffers();
    void invalidateBuffers();
    
private:
    std::mutex _mutex;
    static std::unique_ptr<GfxProgram> _program;
    
    // Buffers containing render data
    bool _dirty;
    float _halfWidth;
    float _locX, _locY;
    Color _color;
    std::vector<Vertex> _points;
    std::vector<Vertex> _mesh;
};

