#pragma once

#include "Scene.hpp"

#include "ImageView.hpp"

class ConfigCodeScene : public Scene
{
 public:
    ConfigCodeScene(MapState& map);
    ~ConfigCodeScene();
    
    const char* SceneName() override;
    const char* SceneResourceDir() override;
    
private:
    ImageView _qrCode;
};

