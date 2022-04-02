#include "ConfigCodeScene.hpp"
#include "LoadShaders.hpp"

#include <chrono>
#include <ctime>


ConfigCodeScene::ConfigCodeScene(MapState& map) : Scene(map, SceneType::Base, SceneLifetime::Manual),
    _qrCode(map)
{
  Elements.push_back(&_qrCode);
  _qrCode.SetImage(ImageRGBA::FromQrPayload("http://bubbulon.com"));
  _qrCode.SetScale(1.0f);
  _qrCode.SetPosition(map.width / 2.0f - _qrCode.GetWidth(), 
                      map.height / 2.0f - _qrCode.GetHeight());
}

ConfigCodeScene::~ConfigCodeScene()
{
}

const char* ConfigCodeScene::SceneName()
{
  return "Config Code Overlay";
}

const char* ConfigCodeScene::SceneResourceDir()
{
  return "ConfigCode";
}
