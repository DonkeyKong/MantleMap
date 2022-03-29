#pragma once

#include "MapState.hpp"

class SceneElement
{
 public:
    virtual ~SceneElement();
    virtual void Draw() final;
protected:
    SceneElement(MapState& map);
    virtual void drawInternal() = 0;
    virtual void initGL() = 0;
    MapState& map;
};
