#ifndef CMDDEBUGSCENE_HPP
#define CMDDEBUGSCENE_HPP

#include "EGL/egl.h"
#include "EGL/eglplatform.h"
#include "GLES2/gl2.h"
#include "EGL/eglext.h"

#include "LoadShaders.hpp"
#include "Scene.hpp"
#include "TextLabel.hpp"
#include "MapState.hpp"

class CmdDebugScene : public Scene
{
 public:
    CmdDebugScene(MapState& map);
    ~CmdDebugScene();
    
    const char* SceneName() override;
    const char* SceneResourceDir() override;
    
    void ShowCmd(std::string query);
    
protected:
    void initGLOverride() override;
    void updateOverride() override;
    void drawOverride() override;
    
private:
    TextLabel _cmdLabel;
    int _cmdShowCounter;
    int _framesToHoldCmd;
    int _framesToScrollCmd;
};

#endif
