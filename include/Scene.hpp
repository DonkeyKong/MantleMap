#pragma once

#include "LoadShaders.hpp"
#include "NaturalEarth.hpp"
#include "MapState.hpp"

#include "EGL/egl.h"
#include "EGL/eglplatform.h"
#include "GLES2/gl2.h"
#include "EGL/eglext.h"

#include <chrono>
#include <string>
#include <math.h>

enum class SceneLifetime
{
  Timed,  // Autohides after a fixed amount of time
  Reset,  // Autohides on reset event
  Manual  // Hides only when subclass logic hides it or if Hide() is specifically called
};

enum class SceneType
{
  Base,
  Animator,
  Overlay
};

class Scene
{
 public:
    // No GL calls allowed here
    Scene(MapState& map, SceneType sceneType, SceneLifetime sceneLifetime);
    virtual ~Scene();
    
    // Provide a user readable or TTS speakabe name for this scene
    virtual const char* SceneName() = 0;
    
    // What is the name of the folder inside "Scenes" where this scene's resouces are stored
    virtual const char* SceneResourceDir() = 0;
    
    // Get 
    virtual std::string GetResourcePath(std::string resourceName) final;
    
    // Initialize all OpenGL data like textures and shaders
    virtual void InitGL() final;
    
    // Reset the scene to its default state
    // For example, you might ask the light map to show to a date time
    // and after some time, it should return to its default state showing 
    // the current time.
    virtual void Reset(bool animate) final;
    
    // Query the scene to perform an action. Returns true if handled, false if not
    virtual bool Query(std::string query, std::string& response);
    
    // Called when scene is shown. Sets visibility to true and prepares other variables
    virtual void Show() final;
    
    // Update any animation variables and fetch any new data
    virtual void Update() final;
    
    // Called when scene is about to be hidden. Sets visibility false.
    virtual void Hide() final;
    
    // Called when base scene has changed
    virtual void BaseSceneChanged(std::string baseSceneName) final;
    
    // Draw the scene to the current OpenGL context
    virtual void Draw() final;
    
    virtual bool Visible() final;
    
    virtual SceneType GetSceneType() final;
    
  protected:
  
    // Overrides for subclasses to customize behavior
    virtual void initGLOverride();
    virtual void drawOverride();
    virtual void updateOverride();
    virtual void showOverride();
    virtual void hideOverride();
    virtual void baseSceneChangedOverride(std::string);
    virtual void resetOverride(bool animate);
  
    // Helper function that draws a fullscreen rect, generally used to draw the map
    void drawMapRect();
    
    // Load an image using image magick and insert it straight into a texture
    GLuint loadImageToTexture(std::string resourceName);
    
    // Load a vert and frag shader and create a program with them
    GfxProgram loadGraphicsProgram(std::string vertShaderName, std::string fragShaderName);

    NaturalEarth projection;
    
    GLuint LonLatLookupTexture;
    MapState& Map;
    std::string BaseSceneName;
    
    // Animation Help Functions
    template<typename T>
    static void moveTowards2D(T& x, T& y, const T& targetX, const T& targetY, T velocity)
    {
      // Get distance
      T dist = sqrt(pow(x-targetX,2.0) + pow(y-targetY,2.0));
  
      if (dist < velocity)
      { 
        x = targetX;
        y = targetY;
      }
      else
      {
        x += (targetX - x) / dist * velocity;
        y += (targetY - y) / dist * velocity;
      }
    }
    
    
    
    // Animation Help Functions
    
    template<typename T>
    static void normAngleDeg(T& a)
    {
      while (a > 180.0)
        a -= 360.0;
      while (a < -180.0)
        a += 360.0;
    }
    
    template<typename T>
    static T angleDiff( T angle1, T angle2 )
    {
      T diff = fmod(( angle2 - angle1 + 180.0 ) , 360.0) - 180.0;
      return diff < -180.0 ? diff + 360.0 : diff;
    }
    
    template<typename T>
    static void moveTowardsAngleDeg2D(T& x, T& y, const T& targetX, const T& targetY, T velocity)
    {
      // Get distance
      T dist = sqrt(pow(angleDiff(x, targetX),2.0) + pow(angleDiff(y, targetY),2.0));
  
      if (dist < velocity)
      { 
        x = targetX;
        y = targetY;
      }
      else
      {
        x += angleDiff(x, targetX) / dist * velocity;
        y += angleDiff(y, targetY) / dist * velocity;
      }
      
      normAngleDeg(x); normAngleDeg(y);
    }
    
    template<typename T>
    static void moveTowards(T& v, const T& targetV, T velocity)
    {
      // Get distance
      T dist = abs(v-targetV);
  
      if (dist < velocity)
      { 
        v = targetV;
      }
      else
      {
        v += (targetV - v) / dist * velocity;
      }
    }
    
  private:
    SceneLifetime _sceneLifetime;
    SceneType _sceneType;
    timepoint_seconds_t _showTime;
    fractionalSeconds _sceneLifetimeSeconds;
    bool _isVisible;
    GLfloat fullscreen_rect_vertex_buffer_data[12];
    
};

