#ifndef DEBUGTRANSFORMSCENE_HPP
#define DEBUGTRANSFORMSCENE_HPP

#include "EGL/egl.h"
#include "EGL/eglplatform.h"
#include "GLES2/gl2.h"
#include "EGL/eglext.h"

#include "LoadShaders.hpp"
#include "Scene.hpp"
#include "MapState.hpp"
#include "TextLabel.hpp"

class DebugTransformScene : public Scene
{
 public:
    DebugTransformScene(MapState& map);
    ~DebugTransformScene();
    
    const char* SceneName() override;
    const char* SceneResourceDir() override;

protected:
    void initGLOverride() override;
    void drawOverride() override;
    
private:
    GfxProgram program;
    std::string vertShader = "vertshader.glsl"; 
    std::string fragShader = "debugfragshader.glsl";
    
    TextLabel _label1;
    TextLabel _label2;
    TextLabel _label3;
    TextLabel _label4;
    TextLabel _label5;
    TextLabel _label6;
    TextLabel _label7;
    TextLabel _label8;
};

#endif
