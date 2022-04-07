#pragma once

#include "ConfigService.hpp"

class SceneElement
{
 public:
    virtual ~SceneElement();
    virtual void Draw() final;
protected:
    SceneElement(ConfigService& map);
    virtual void drawInternal() = 0;
    virtual void initGL() = 0;
    ConfigService& map;
};
