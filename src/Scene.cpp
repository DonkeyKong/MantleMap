#include "Scene.hpp"

#include <assert.h>
#include <filesystem>

#define check() assert(glGetError() == 0)

Scene::Scene(MapState& map, SceneType sceneType, SceneLifetime sceneLifetime) : 
  Map(map),
  projection(map),
  fullscreen_rect_vertex_buffer_data
  { 
    0.0f, (float)map.height, 0.0f,
    0.0f,  0.0f, 0.0f,
    (float)map.width, (float)map.height, 0.0f,
    (float)map.width,  0.0f, 0.0f
  }
{
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
  auto filePath = std::filesystem::path(Map.sceneResourcePath) / SceneResourceDir() / resourceName;
  if (std::filesystem::exists(filePath))
  {
    return filePath;
  }
  filePath = std::filesystem::path(Map.sceneResourcePath) / "Shared" / resourceName;
  if (std::filesystem::exists(filePath))
  {
    return filePath;
  }
  return std::string();
}

void Scene::InitGL()
{
  // Create the LonLatLookupTexture
  std::vector<unsigned char> lut = projection.getInvLookupTable();
  
  glGenTextures(1, &LonLatLookupTexture);
  glBindTexture(GL_TEXTURE_2D, LonLatLookupTexture);
  glTexImage2D(GL_TEXTURE_2D, 0 , GL_RGBA, Map.width, Map.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &lut[0]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  check();
  
  initGLOverride();
  check();
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
    drawOverride();
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
// Do nothing, child objects should override
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

void Scene::drawMapRect()
{
  glVertexAttribPointer(
                        0, //vertexPosition_modelspaceID, // The attribute we want to configure
                        3,                  // size
                        GL_FLOAT,           // type
                        GL_FALSE,           // normalized?
                        0,                  // stride
                        fullscreen_rect_vertex_buffer_data // (void*)0            // array buffer offset
                );

   // see above glEnableVertexAttribArray(vertexPosition_modelspaceID);
   glEnableVertexAttribArray ( 0 );

  // Draw the triangles!
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}