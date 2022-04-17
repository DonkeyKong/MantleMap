#include "SceneElement.hpp"
#include "ConfigService.hpp"
static auto& config = ConfigService::global;

SceneElement::SceneElement()
{
    // Nothing to do
}

SceneElement::~SceneElement()
{
    // Nothing to do
}

void SceneElement::Draw()
{
    initGL();
    drawInternal();
}

// Load an image using libpng and insert it straight into a texture
std::unique_ptr<GfxTexture> SceneElement::loadTexture(std::string resourceName)
{
    return std::make_unique<GfxTexture>(config.GetSharedResourcePath(resourceName));
}

// Load a vert and frag shader and create a program with them
std::unique_ptr<GfxProgram> SceneElement::loadProgram(std::string vertShaderName, std::string fragShaderName, std::vector<std::string> features)
{
    return std::make_unique<GfxProgram>(config.GetSharedResourcePath(vertShaderName), config.GetSharedResourcePath(fragShaderName), features);
}
