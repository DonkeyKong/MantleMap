#ifndef LIGHTSCENE_HPP
#define LIGHTSCENE_HPP

#include "EGL/egl.h"
#include "EGL/eglplatform.h"
#include "GLES2/gl2.h"
#include "EGL/eglext.h"

#include "LoadShaders.hpp"
#include "Scene.hpp"
#include "MapState.hpp"

class LightScene : public Scene
{
 public:
    LightScene(MapState& map);
    ~LightScene();
    
    const char* SceneName() override;
    const char* SceneResourceDir() override;
    bool Query(std::string query, std::string& response) override;

protected:
    void initGLOverride() override;
    void resetOverride(bool animate) override;
    void updateOverride() override;
    void drawOverride() override;
    
private:
    GfxProgram program;
    GLuint mapLayer1Texture;
    GLuint mapLayer2Texture;
    std::string mapImagePath = "map_day.png";
    std::string mapImageNightPath = "map_night.png";
    std::string vertShader = "vertshader.glsl"; 
    std::string fragShader = "lightfragshader.glsl";

    double sunTargetLat;
    double sunTargetLon;
    float sunPropAngleCurrent;
    float sunPropAngleTarget;
    double sunCurrentLat;
    double sunCurrentLon;
    bool overrideSunLocation;
};

#endif
