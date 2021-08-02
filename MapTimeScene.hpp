#ifndef MAPTIMESCENE_HPP
#define MAPTIMESCENE_HPP

#include "Scene.hpp"

#include "TextLabel.hpp"

class MapTimeScene : public Scene
{
 public:
    MapTimeScene(MapState& map);
    ~MapTimeScene();
    
    const char* SceneName() override;
    const char* SceneResourceDir() override;
    
protected:
    void initGLOverride() override;
    void updateOverride() override;
    void drawOverride() override;
    
private:
    TextLabel _monthLabel;
    TextLabel _dayLabel;
    TextLabel _yearLabel;
    TextLabel _timeLabel;
};

#endif
