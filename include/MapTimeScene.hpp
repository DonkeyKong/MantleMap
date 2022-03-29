#pragma once

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
    void updateOverride() override;
    
private:
    TextLabel _monthLabel;
    TextLabel _dayLabel;
    TextLabel _yearLabel;
    TextLabel _timeLabel;
};

