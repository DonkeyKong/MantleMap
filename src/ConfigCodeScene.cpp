#include "ConfigCodeScene.hpp"
#include "GfxProgram.hpp"

#include <fmt/format.h>
#include <chrono>
#include <ctime>


ConfigCodeScene::ConfigCodeScene(ConfigService& config, HttpService& http) : Scene(config, SceneType::Base, SceneLifetime::Manual),
    qrCode(config),
    scanToConfigureLabel(config),
    urlLabel(config)
{
  Elements.push_back(&scanToConfigureLabel);
  Elements.push_back(&urlLabel);
  Elements.push_back(&qrCode);

  // Get the URL to show from the HttpService
  std::string configURL = fmt::format("http://{}", http.ListeningInterface());

  qrCode.SetImage(ImageRGBA::FromQrPayload(configURL));
  qrCode.SetScale(1.0f);
  qrCode.SetColor(0.5, 0.5, 0.5, 1.0);
  qrCode.SetPosition(config.width / 2.0f -  qrCode.GetWidth() / 2.0f, 
                     config.height / 2.0f - qrCode.GetHeight() / 2.0f);

  scanToConfigureLabel.SetText("Scan to Connect");
  scanToConfigureLabel.SetFontStyle(FontStyle::BigNarrow);
  scanToConfigureLabel.SetAlignment(HAlign::Center);
  scanToConfigureLabel.SetPosition(config.width / 2.0f, config.height / 2.0f - 32);
  scanToConfigureLabel.SetColor(0.5, 0.5, 0.5, 1.0);
  
  urlLabel.SetText(configURL);
  urlLabel.SetFontStyle(FontStyle::Narrow);
  urlLabel.SetAlignment(HAlign::Center);
  urlLabel.SetPosition(config.width / 2.0f, config.height / 2.0f + 22);
  urlLabel.SetColor(0.5, 0.5, 0.5, 1.0);
}

ConfigCodeScene::~ConfigCodeScene()
{
}

const char* ConfigCodeScene::SceneName()
{
  return "Config Code";
}

const char* ConfigCodeScene::SceneResourceDir()
{
  return "ConfigCode";
}
