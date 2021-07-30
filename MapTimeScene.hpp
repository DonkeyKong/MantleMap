#ifndef MAPTIMESCENE_HPP
#define MAPTIMESCENE_HPP

#include "EGL/egl.h"
#include "EGL/eglplatform.h"
#include "GLES2/gl2.h"
#include "EGL/eglext.h"

#include "LoadShaders.hpp"
#include "Scene.hpp"
#include "TextLabel.hpp"
#include "MapState.hpp"

class MapTimeScene : public Scene
{
 public:
    MapTimeScene(MapState& map);
    ~MapTimeScene();
    
    const char* SceneName() override;
    const char* SceneResourceDir() override;
    
protected:
    void updateOverride() override;
    void drawOverride() override;
    
private:
    TextLabel _monthLabel;
    TextLabel _dayLabel;
    TextLabel _yearLabel;
    TextLabel _timeLabel;
};

#endif
