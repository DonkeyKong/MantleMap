#include "MapTimeScene.hpp"

#include <chrono>
#include <ctime>


MapTimeScene::MapTimeScene() : Scene(SceneType::Overlay, SceneLifetime::Manual)
{
  Elements.push_back(&_timeLabel);
	Elements.push_back(&_monthLabel);
	Elements.push_back(&_dayLabel);
	Elements.push_back(&_yearLabel);
}

MapTimeScene::~MapTimeScene()
{
}

const char* MapTimeScene::SceneName()
{
  return "MapTime";
}

void MapTimeScene::updateOverride()
{
  auto nowLocal = TimeService::GetSceneTimeAsLocaltime();
  char formatStr[256];
  
  if (BaseSceneName == "Solar")
  {
    // Setup the time label
    std::strftime(formatStr, 255, "%H:%M", &nowLocal );
    _timeLabel.SetText(formatStr);
    _timeLabel.SetFontStyle(FontStyle::Regular, 2.0f);
    _timeLabel.SetColor(0.5, 0.5, 0.5, 1.0);
    _timeLabel.SetPosition(96, 74);
    _timeLabel.SetAlignment(HAlign::Center);
    
    // Setup the date label
    std::strftime(formatStr, 255, "%B %d, %Y", &nowLocal );
    _monthLabel.SetText(formatStr);
    _monthLabel.SetFontStyle(FontStyle::Narrow);
    _monthLabel.SetColor(0.4, 0.4, 0.4, 1.0);
    _monthLabel.SetPosition(96, 88);
    _monthLabel.SetAlignment(HAlign::Center);
    
    // Disable Month and Year labels
    _dayLabel.SetText("");
    _yearLabel.SetText("");
  }
  else
  {
    // Time label
    std::strftime(formatStr, 255, "%H:%M", &nowLocal );
    _timeLabel.SetText(formatStr);
    _timeLabel.SetFontStyle(FontStyle::Regular);
    _timeLabel.SetColor(0.5,0.5,0.5f,1.0f);
    _timeLabel.SetPosition(188, 86);
    _timeLabel.SetAlignment(HAlign::Right);
    
    // Month Label
    std::strftime(formatStr, 255, "%B", &nowLocal );
    if (strlen(formatStr) > 7)
      std::strftime(formatStr, 255, "%b", &nowLocal );
    _monthLabel.SetText(formatStr);
    _monthLabel.SetFontStyle(FontStyle::Narrow);
    _monthLabel.SetColor(0.5,0.5,0.5f,1.0f);
    _monthLabel.SetPosition(188, 4);
    _monthLabel.SetAlignment(HAlign::Right);
    
    // Day label
    std::strftime(formatStr, 255, "%d", &nowLocal );
    _dayLabel.SetText(formatStr);
    _dayLabel.SetColor(0.5,0.5,0.5f,1.0f);
    _dayLabel.SetPosition(188, 10);
    _dayLabel.SetAlignment(HAlign::Right);

    // Year Label
    std::strftime(formatStr, 255, "%Y", &nowLocal );
    _yearLabel.SetText(formatStr);
    _yearLabel.SetFontStyle(FontStyle::Narrow);
    _yearLabel.SetColor(0.5,0.5,0.5f,1.0f);
    _yearLabel.SetPosition(188, 16);
    _yearLabel.SetAlignment(HAlign::Right);
  }
}
