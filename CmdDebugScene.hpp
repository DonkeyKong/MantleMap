#ifndef CMDDEBUGSCENE_HPP
#define CMDDEBUGSCENE_HPP

#include "Scene.hpp"
#include "TextLabel.hpp"

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
