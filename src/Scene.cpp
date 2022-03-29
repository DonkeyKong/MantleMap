#include "Scene.hpp"

#include <assert.h>
#include <filesystem>

#define check() assert(glGetError() == 0)

Scene::Scene(MapState& map, SceneType sceneType, SceneLifetime sceneLifetime) : 
  map(map)
{
  _initGLDone = false;
  _isVisible = false;
  _sceneType = sceneType;
  _sceneLifetime = sceneLifetime;
  _sceneLifetimeSeconds = fractionalSeconds(10.0);
}

Scene::~Scene()
{
  // Nothing to do here because we have no dynamically allocated resources
}

std::string Scene::GetResourcePath(std::string resourceName)
{
  auto filePath = std::filesystem::path(map.sceneResourcePath) / SceneResourceDir() / resourceName;
  if (std::filesystem::exists(filePath))
  {
    return filePath;
  }
  filePath = std::filesystem::path(map.sceneResourcePath) / "Shared" / resourceName;
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
    check();
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
    check();
    drawOverride();
    check();
  }
}

void Scene::BaseSceneChanged(std::string baseSceneName)
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
    check();
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

bool Scene::Query(std::string query, std::string& response)
{
  // By default don't accept any queries
  return false;
}

GLuint Scene::loadImageToTexture(std::string resourceName)
{
  return LoadImageToTexture(GetResourcePath(resourceName));
}

GfxProgram Scene::loadGraphicsProgram(std::string vertShaderName, std::string fragShaderName)
{
  return LoadGraphicsProgram(GetResourcePath(vertShaderName), GetResourcePath(fragShaderName));
}