#pragma once

#include "ConfigService.hpp"
#include "GfxTexture.hpp"
#include "GfxProgram.hpp"

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
    std::unique_ptr<GfxTexture> loadTexture(std::string resourceName);
    std::unique_ptr<GfxProgram> loadProgram(std::string vertShaderName, std::string fragShaderName, std::vector<std::string> features);
};
