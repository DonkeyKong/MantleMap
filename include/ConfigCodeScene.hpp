#pragma once

#include "Scene.hpp"
#include "HttpService.hpp"

#include "ImageView.hpp"
#include "TextLabel.hpp"

class ConfigCodeScene : public Scene
{
 public:
    ConfigCodeScene(ConfigService& config, HttpService& http);
    ~ConfigCodeScene();
    
    const char* SceneName() override;
    const char* SceneResourceDir() override;
    
private:
    ImageView qrCode;
    TextLabel scanToConfigureLabel;
    TextLabel urlLabel;
};

