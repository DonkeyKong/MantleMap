#include "WeatherScene.hpp"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "cpp-httplib/httplib.h"
#include <nlohmann/json.hpp>
#include <fmt/format.h>
using json = nlohmann::json;

#include <math.h>
#include <chrono>
#include <string>

static const char degChar[] {0xB0, 0x00};
static const std::string deg(degChar);
static const int INVALID_TEMP = -999;
static const double earthRadiusKm = 6371;

static double deg2rad(double deg)
{
  return deg * (M_PI / 180.0);
}

static double gpsDistKm(double latADeg, double lonADeg, double latBDeg, double lonBDeg)
{
  double lat1r, lon1r, lat2r, lon2r, u, v;
  lat1r = deg2rad(latADeg);
  lon1r = deg2rad(lonADeg);
  lat2r = deg2rad(latBDeg);
  lon2r = deg2rad(lonBDeg);
  u = sin((lat2r - lat1r)/2);
  v = sin((lon2r - lon1r)/2);
  return 2.0 * earthRadiusKm * asin(sqrt(u * u + cos(lat1r) * cos(lat2r) * v * v));
}

static int getStationCurrentTemp(httplib::Client& client, const std::string& stationID)
{
  try
  {
    auto observationJson = json::parse(client.Get(fmt::format("/stations/{}/observations/latest", stationID).c_str())->body);
    float temp = observationJson["properties"]["temperature"]["value"];
    return (int)round(temp);
  }
  catch (...)
  {
  }
  return INVALID_TEMP;
}

WeatherScene::WeatherScene(MapState& map) : Scene(map, SceneType::Overlay, SceneLifetime::Manual),
  _tempLabel(map)
{
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
    
    try
    {
      _noaaTemp = "Req";
      
      // Request the point info for home zone of home
      auto pointsJson = json::parse(
                          noaa.Get(
                            fmt::format("/points/{0:.4f},{1:.4f}", Map.homeLatitudeDeg, Map.homeLongitudeDeg).c_str()
                          )->body);
      std::string stationsURL = pointsJson["properties"]["observationStations"];
      _noaaTemp = "Pts";
      
      // Request info on the observation stations
      auto stationsJson = json::parse(noaa.Get(stationsURL.c_str())->body);
      _noaaTemp = "Sta";
      
      // Get the closest station
      double minDist = earthRadiusKm*2.0;
      for (auto station : stationsJson["features"])
      {
        double statLat = station["geometry"]["coordinates"][1];
        double statLon = station["geometry"]["coordinates"][0];
        double dist = gpsDistKm(Map.homeLatitudeDeg, Map.homeLongitudeDeg,
                              statLat, statLon);
        std::string id = station["properties"]["stationIdentifier"];
        
        if (dist < minDist && getStationCurrentTemp(noaa, id) != INVALID_TEMP)
        {
          minDist = dist;
          stationID = id;
        }
      }
      _noaaTemp = stationID;
    }
    catch (...)
    {
      return;
    }
    
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

void WeatherScene::initGLOverride()
{
  TextLabel::InitGL(Map);
}

void WeatherScene::updateOverride()
{
  _tempLabel.SetText(_noaaTemp);
  _tempLabel.SetFontStyle(FontStyle::Regular);
  _tempLabel.SetColor(0.5,0.5,0.5f,1.0f);
  _tempLabel.SetPosition(4, 4);
}

void WeatherScene::drawOverride()
{
  _tempLabel.Draw();
}
