#include "Scene.hpp"

#include <assert.h>
#include <filesystem>

#include "GLError.hpp"

Scene::Scene(ConfigService& map, SceneType sceneType, SceneLifetime sceneLifetime) : 
  config(map)
{
  _initGLDone = false;
  _isVisible = false;
  _sceneType = sceneType;
  _sceneLifetime = sceneLifetime;
  _sceneLifetimeSeconds = fractionalSeconds(10.0);
  clearBeforeDraw = true;
}

Scene::~Scene()
{
  // Nothing to do here because we have no dynamically allocated resources
}

std::string Scene::GetResourcePath(std::string resourceName)
{
  auto filePath = std::filesystem::path(config.sceneResourcePath) / SceneName() / resourceName;
  if (std::filesystem::exists(filePath))
  {
    return filePath;
  }
  filePath = std::filesystem::path(config.sceneResourcePath) / "Shared" / resourceName;
  if (std::filesystem::exists(filePath))
  {
    return filePath;
  }
  return std::string();
}

void Scene::initGL()
{
  if (!_initGLDone)
  {    
    initGLOverride();
    print_if_glerror("InitGL for scene " << SceneName());
    _initGLDone = true;
  }
}

void Scene::Reset(bool animate)
{
  if (_sceneLifetime == SceneLifetime::Reset)
    Hide();
  resetOverride(animate);
}

// Called when scene is shown. Sets visibility to true and prepares other variables
void Scene::Show()
{
  if (!_isVisible)
  {
    _isVisible = true;
    _showTime = std::chrono::system_clock::now();
    showOverride();
  }
}

// Update any animation variables and fetch any new data
void Scene::Update()
{
  if (_isVisible)
  {
    if (_sceneLifetime == SceneLifetime::Timed)
    {
      timepoint_seconds_t now = std::chrono::system_clock::now();
      if (now - _showTime > _sceneLifetimeSeconds)
      {
        Hide();
        return;
      }
    }
    
    updateOverride();
  }
}

// Called when scene is about to be hidden. Sets visibility false.
void Scene::Hide()
{
  if (_isVisible)
  {
    _isVisible = false;
    hideOverride();
  }
}

// Draw the scene to the current OpenGL context
void Scene::Draw()
{
  if (_isVisible)
  {
    initGL();
    print_if_glerror("InitGL for scene " << SceneName());

    if (_sceneType == SceneType::Base && clearBeforeDraw)
    {
        // Clear the whole buffer
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear( GL_COLOR_BUFFER_BIT );
    }

    drawOverride();
    print_if_glerror("Draw for scene " << SceneName());
  }
}

void Scene::OnSceneChanged(std::string baseSceneName)
{
  BaseSceneName = baseSceneName;
  baseSceneChangedOverride(BaseSceneName);
}

void Scene::initGLOverride()
{
  // Do nothing, child objects should override
}

void Scene::drawOverride()
{
  // Default behavior is just to draw all children
  for (auto element : Elements)
  {
    element->Draw();
    print_if_glerror("Draw for element in scene " << SceneName());
  }
}

void Scene::updateOverride()
{
  // Do nothing, child objects should override
}

void Scene::showOverride()
{
  // Do nothing, child objects should override
}

void Scene::hideOverride()
{
  // Do nothing, child objects should override
}

void Scene::resetOverride(bool animate)
{
  // Do nothing, child objects should override
}

void Scene::baseSceneChangedOverride(std::string baseSceneName)
{
  // Do nothing, child objects may override
}

bool Scene::Visible()
{
  return _isVisible;
}

SceneType Scene::GetSceneType()
{
  return _sceneType;
}

// Load an image using libpng and insert it straight into a texture
std::unique_ptr<GfxTexture> Scene::loadTexture(std::string resourceName)
{
    return std::make_unique<GfxTexture>(GetResourcePath(resourceName));
}

// Load a vert and frag shader and create a program with them
std::unique_ptr<GfxProgram> Scene::loadProgram(std::string vertShaderName, std::string fragShaderName, std::vector<std::string> features)
{
    return std::make_unique<GfxProgram>(config, GetResourcePath(vertShaderName), GetResourcePath(fragShaderName), features);
}