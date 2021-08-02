#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
#include <iostream>
#include <bcm_host.h>

#include <regex>
#include <algorithm>
#include <string>
#include <sstream>
#include <chrono>
#include <ctime>

#include "MapTimeScene.hpp"

MapTimeScene::MapTimeScene(MapState& map) : Scene(map, SceneType::Overlay, SceneLifetime::Manual),
    _monthLabel(map),
    _dayLabel(map),
    _yearLabel(map),
    _timeLabel(map)
{
  
}

MapTimeScene::~MapTimeScene()
{
}

void MapTimeScene::initGLOverride()
{
  TextLabel::InitGL(Map);
}

const char* MapTimeScene::SceneName()
{
  return "Map Time Overlay";
}

const char* MapTimeScene::SceneResourceDir()
{
  return "MapTime";
}

void MapTimeScene::updateOverride()
{
  auto nowLocal = Map.GetMapTimeAsLocaltime();
  char formatStr[256];
  
  if (BaseSceneName == "Solar")
  {
    // Setup the time label
    std::strftime(formatStr, 255, "%H:%M", &nowLocal );
    _timeLabel.SetText(formatStr);
    _timeLabel.SetFontStyle(FontStyle::Regular, 2.0f);
    _timeLabel.SetColor(0.5, 0.5, 0.5, 1.0);
    _timeLabel.SetPosition(96, 74);
    _timeLabel.SetAlignment(TextAlignment::Center);
    
    // Setup the date label
    std::strftime(formatStr, 255, "%B %d, %Y", &nowLocal );
    _monthLabel.SetText(formatStr);
    _monthLabel.SetFontStyle(FontStyle::Narrow);
    _monthLabel.SetColor(0.4, 0.4, 0.4, 1.0);
    _monthLabel.SetPosition(96, 88);
    _monthLabel.SetAlignment(TextAlignment::Center);
    
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
    _timeLabel.SetColor(0.25, 0.25, 0.25, 1.0);
    _timeLabel.SetPosition(192, 90);
    _timeLabel.SetAlignment(TextAlignment::AlignToEnd);
    
    // Month Label
    std::strftime(formatStr, 255, "%B", &nowLocal );
    if (strlen(formatStr) > 7)
      std::strftime(formatStr, 255, "%b", &nowLocal );
    _monthLabel.SetText(formatStr);
    _monthLabel.SetFontStyle(FontStyle::Narrow);
    _monthLabel.SetColor(0.25, 0.25, 0.25, 1.0);
    _monthLabel.SetPosition(192, 0);
    _monthLabel.SetAlignment(TextAlignment::AlignToEnd);
    
    // Day label
    std::strftime(formatStr, 255, "%d", &nowLocal );
    _dayLabel.SetText(formatStr);
    _dayLabel.SetColor(0.25, 0.25, 0.25, 1.0);
    _dayLabel.SetPosition(192, 6);
    _dayLabel.SetAlignment(TextAlignment::AlignToEnd);

    // Year Label
    std::strftime(formatStr, 255, "%Y", &nowLocal );
    _yearLabel.SetText(formatStr);
    _yearLabel.SetFontStyle(FontStyle::Narrow);
    _yearLabel.SetColor(0.25, 0.25, 0.25, 1.0);
    _yearLabel.SetPosition(192, 12);
    _yearLabel.SetAlignment(TextAlignment::AlignToEnd);
  }
}

void MapTimeScene::drawOverride()
{
  _timeLabel.Draw();
	_monthLabel.Draw();
	_dayLabel.Draw();
	_yearLabel.Draw();
}
