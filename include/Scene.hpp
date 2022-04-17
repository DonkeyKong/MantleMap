#pragma once

#include "GfxTexture.hpp"
#include "GfxProgram.hpp"

#include "NaturalEarth.hpp"
#include "SceneElement.hpp"
#include "TimeService.hpp"
#include "HttpService.hpp"

// #include "EGL/egl.h"
// #include "EGL/eglplatform.h"
#include "GLES2/gl2.h"
// #include "EGL/eglext.h"

#include <chrono>
#include <string>

enum class SceneLifetime
{
  Timed,  // Autohides after a fixed amount of time
  Reset,  // Autohides on reset event
  Manual  // Hides only when subclass logic hides it or if Hide() is specifically called
};

enum class SceneType
{
  Base,
  Overlay
};

class Scene
{
 public:
    // No GL calls allowed here
    Scene(SceneType sceneType, SceneLifetime sceneLifetime);
    virtual ~Scene();
    
    // What is the name of the folder inside "Scenes" where this scene's resouces are stored
    virtual const char* SceneName() = 0;

    virtual void RegisterEndpoints(HttpService& http) final;
    
    // Get a resource for the scene, first looking in scenes/<name>, then in scenes/Shared
    virtual std::string GetResourcePath(std::string resourceName) final;
    
    // Reset the scene to its default state
    // For example, you might ask the light map to show to a date time
    // and after some time, it should return to its default state showing 
    // the current time.
    virtual void Reset(bool animate) final;
    
    // Called when scene is shown. Sets visibility to true and prepares other variables
    virtual void Show() final;
    
    // Update any animation variables and fetch any new data
    virtual void Update() final;
    
    // Called when scene is about to be hidden. Sets visibility false.
    virtual void Hide() final;
    
    // Called when base scene has changed
    virtual void OnSceneChanged(std::string baseSceneName) final;
    
    // Draw the scene to the current OpenGL context
    virtual void Draw() final;
    
    virtual bool Visible() final;
    
    virtual SceneType GetSceneType() final;
    
  protected:
    // Overrides for subclasses to customize behavior
    virtual void initGLOverride();
    virtual void registerEndpointsOverride(HttpService& http);
    virtual void drawOverride();
    virtual void updateOverride();
    virtual void showOverride();
    virtual void hideOverride();
    virtual void baseSceneChangedOverride(std::string);
    virtual void resetOverride(bool animate);

    // Load an image using libpng and insert it straight into a texture
    std::unique_ptr<GfxTexture> loadTexture(std::string resourceName);
    
    // Load a vert and frag shader and create a program with them
    std::unique_ptr<GfxProgram> loadProgram(std::string vertShaderName, std::string fragShaderName, std::vector<std::string> features);

    std::vector<SceneElement*> Elements;
    std::string BaseSceneName;
    bool clearBeforeDraw;
    
  private:
    // Initialize all OpenGL data like textures and shaders
    virtual void initGL() final;
    bool _initGLDone;

    SceneLifetime _sceneLifetime;
    SceneType _sceneType;
    timepoint_seconds_t _showTime;
    fractionalSeconds _sceneLifetimeSeconds;
    bool _isVisible;
};

