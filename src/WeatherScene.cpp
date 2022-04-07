#include "WeatherScene.hpp"
#include "AstronomyService.hpp"

#include <httplib.h>
#include <nlohmann/json.hpp>
#include <fmt/format.h>
using json = nlohmann::json;

#include <math.h>
#include <chrono>
#include <string>

static const unsigned char degChar[] {0xB0, 0x00};
static const std::string deg((char*)degChar);
static const int INVALID_TEMP = -999;

static json getJsonPayload(httplib::Client& client, const std::string& url)
{
  // Get an httplib result by getting the URL in question
  auto result = client.Get(url.c_str());

  // If the result is non-null and the HTTP status is 2xx, then parse the json
  if (result && result->status >= 200 && result->status < 300)
  {
      return json::parse(result->body);
  }

  throw std::runtime_error("Error fetching HTTP request!");
}

static int getStationCurrentTemp(httplib::Client& client, const std::string& stationID)
{
  try
  {
    auto observationJson = getJsonPayload(client, fmt::format("/stations/{}/observations/latest", stationID));
    float temp = observationJson["properties"]["temperature"]["value"];
    return (int)round(temp);
  }
  catch (...)
  {
  }
  return INVALID_TEMP;
}

WeatherScene::WeatherScene(ConfigService& config) : Scene(config, SceneType::Overlay, SceneLifetime::Manual),
  _tempLabel(config)
{
  // Add child elements...
  Elements.push_back(&_tempLabel);

  _noaaTemp = "Initializing";
  _exitTempUpdateThread = false;
  
  _tempUpdateThread = std::make_shared<std::thread>([&]()
  { 
    // Create a client to noaa's API
    httplib::Client noaa("https://api.weather.gov");
    noaa.set_default_headers({
      { "User-Agent", "(MantleMap, admin@bubbulon.com)" },
      { "Accept", "application/geo+json" }
    });
    
    std::string stationID = "????";
    //bool stationFound = false;

    //while (!stationFound)
    //{
      try
      {
        _noaaTemp = "Req";
        
        // Request the point info for home zone of home
        auto pointsJson = getJsonPayload(noaa, 
          fmt::format("/points/{0:.4f},{1:.4f}", config.homeLatitudeDeg, config.homeLongitudeDeg));

        std::string stationsURL = pointsJson["properties"]["observationStations"];
        _noaaTemp = "Pts";
        
        // Request info on the observation stations
        auto stationsJson = getJsonPayload(noaa, stationsURL);
        _noaaTemp = "Sta";
        
        // Get the closest station
        double minDist = AstronomyService::EarthRadiusKm*2.0;
        for (auto station : stationsJson["features"])
        {
          double statLat = station["geometry"]["coordinates"][1];
          double statLon = station["geometry"]["coordinates"][0];
          double dist = AstronomyService::GpsDistKm(config.homeLatitudeDeg, config.homeLongitudeDeg,
                                statLat, statLon);
          std::string id = station["properties"]["stationIdentifier"];
          
          if (dist < minDist && getStationCurrentTemp(noaa, id) != INVALID_TEMP)
          {
            minDist = dist;
            stationID = id;
          }
        }
        _noaaTemp = stationID;
        //stationFound = true;
      }
      catch (...)
      {
        return;
      }

      // if (!stationFound)
      // {
      //   std::unique_lock<std::mutex> lock(_updateThreadMutex);
      //   _exitThreadCondition.wait_for( 
      //                   lock,
      //                   std::chrono::seconds(60));
      // }
    //}
    
    // In a loop, get temperature observations!
    int errorCount = 0;
    while (!_exitTempUpdateThread)
    {
      int temp = getStationCurrentTemp(noaa, stationID);
      if (temp == INVALID_TEMP)
      {
        _noaaTemp = fmt::format("Err {} @ {}", errorCount++, stationID);
        if (errorCount > 255)
          return;
      }
      else
      {
        _noaaTemp = fmt::format("{}{}C", temp, deg);
        errorCount = 0;
      }
      
      {
        std::unique_lock<std::mutex> lock(_updateThreadMutex);
        _exitThreadCondition.wait_for( 
                        lock,
                        std::chrono::seconds(60));
      }
      
      //std::this_thread::sleep_for (std::chrono::seconds(60));
    }
  });
}

WeatherScene::~WeatherScene()
{ 
  {
     _exitTempUpdateThread = true;
     std::lock_guard<std::mutex> lock(_updateThreadMutex);
     _exitThreadCondition.notify_one();
  }
  
  _tempUpdateThread->join();
}

const char* WeatherScene::SceneName()
{
  return "Weather";
}

const char* WeatherScene::SceneResourceDir()
{
  return "Weather";
}

void WeatherScene::updateOverride()
{
  _tempLabel.SetText(_noaaTemp);
  _tempLabel.SetFontStyle(FontStyle::Regular);
  _tempLabel.SetColor(0.5,0.5,0.5f,1.0f);
  _tempLabel.SetPosition(4, 4);
}

