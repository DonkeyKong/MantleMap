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

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "cpp-httplib/httplib.h"
#include <nlohmann/json.hpp>
#include <fmt/format.h>
using json = nlohmann::json;

#include "SolarScene.hpp"

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
    int temp = observationJson["properties"]["temperature"]["value"];
    return temp;
  }
  catch (...)
  {
  }
  return INVALID_TEMP;
}

SolarScene::SolarScene(MapState& map) : Scene(map, SceneType::Base, SceneLifetime::Manual),
  _solarLine(map), _horizonLine(map), _sunCircle(map), _lunarLine(map), _moonCircle(map),
  _tempLabel(map), _sunriseLabel(map), _sunsetLabel(map)
{   
  _vScale = (double)Map.height / 180.0 * 0.9;
  _vOffset = (double)Map.height * 0.05;
  _hScale = (double)Map.width;
  
  _moonColorDay  = {0.3f,0.3f,0.3f,1.0f};
  _moonColorNight  = {0.6f,0.6f,0.6f,1.0f};
  _sunColorDay  = {1.0f,0.75f,0.0f,1.0f};
  _sunColorNight = {0.15f,0.0f,0.7f,1.0f};
  _skyColorDay = {0.2f,0.3f,0.4f,1.0f};
  _skyColorNight = {0.15f,0.15f,0.25f,1.0f};
  
  _endJulianMoon = 0;
  _endJulian = 0;
  _startJulian = 0;
  _startJulianMoon = 0;
  _sunriseJulian = 0;
  _sunsetJulian = 0;
  
  _horizonLine.AddPoint({ 0, _vOffset + 90.0f * _vScale, 0,
                          0.2f, 0.2f, 0.2f, 1.0f});
  _horizonLine.AddPoint({ 1.0f * _hScale, _vOffset + 90.0f * _vScale, 0,
    0.2f, 0.2f, 0.2f, 1.0f});
  _horizonLine.SetThickness(2.0f);
    
  float sunRadius = 5.0f;
  float moonRadius = 3.0f;
  for (float a = 0; a <= (float)(2.1*M_PI); a+= 0.1f)
  {
    _sunCircle.AddPoint({ sunRadius * sin(a), sunRadius * cos(a), 0,
    1, 1, 1, 1});
    _moonCircle.AddPoint({ moonRadius * sin(a), moonRadius * cos(a), 0,
    1, 1, 1, 1});
  }
  
  _lunarLine.SetThickness(1.5f);
  _solarLine.SetThickness(2.0f);
  
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
      _noaaTemp = "Requesting";
      
      // Request the point info for home zone of home
      auto pointsJson = json::parse(
                          noaa.Get(
                            fmt::format("/points/{0:.4f},{1:.4f}", Map.homeLatitudeDeg, Map.homeLongitudeDeg).c_str()
                          )->body);
      std::string stationsURL = pointsJson["properties"]["observationStations"];
      _noaaTemp = "Points";
      
      // Request info on the observation stations
      auto stationsJson = json::parse(noaa.Get(stationsURL.c_str())->body);
      _noaaTemp = "Stations";
      
      // Get the closest station
      double minDist = earthRadiusKm*2.0;
      for (auto station : stationsJson["features"])
      {
        double statLat = station["geometry"]["coordinates"][1];
        double statLon = station["geometry"]["coordinates"][0];
        double dist = gpsDistKm(Map.homeLatitudeDeg, Map.homeLongitudeDeg,
                              statLat, statLon);
        std::string id = station["properties"]["stationIdentifier"];
        
        //std::cout << id << ": " << dist << std::endl;
        
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
      
      std::this_thread::sleep_for (std::chrono::seconds(60));
    }
  });
}

SolarScene::~SolarScene()
{
  _exitTempUpdateThread = true;
}

const char* SolarScene::SceneName()
{
  return "Solar Graph";
}

const char* SolarScene::SceneResourceDir()
{
  return "Solar";
}

void SolarScene::initGLOverride()
{
  PolyLine::InitGL();
}

static float invInterp(float start, float end, float val)
{
  if (end == start)
    return 0;
    
  float t = (val - start) / (end - start);
  
  if (t > 1.0f)
    return 1.0f;
  if (t < 0.0f)
    return 0.0f;
  return t;
}

static float interpolate(float start, float end, float t)
{
  return start * (1.0f-t) + end * (t);
}

static Color interpolate(Color start, Color end, float t)
{
  return {  start.r * (1.0f-t) + end.r * (t),
            start.g * (1.0f-t) + end.g * (t),
            start.b * (1.0f-t) + end.b * (t),
            start.a * (1.0f-t) + end.a * (t) };
}

void SolarScene::updateOverride()
{
  // Generate today's solar curve if it's stale
  double nowJulian = Map.GetMapTimeAsJulianDate();
  
  if (nowJulian > _endJulian || nowJulian < _startJulian)
  {
    // Get the required julian dates
    auto start = Map.GetMapTimeAsLocaltime();
    start.tm_sec = 0;
    start.tm_min = 0;
    start.tm_hour = 0;
    _startJulian = Map.GetJulianDateFromLocaltime(start);
    _endJulian = _startJulian+1.0;

    // Create the sun polyline
    std::vector<PolyLinePoint> points;
    for (double t = _startJulian; t <= (_endJulian+0.05); t += (4.0/(double)Map.width))
    {
      double latitudeDeg, longitudeDeg;
      AstronomyService.GetSolarPoint(t, latitudeDeg, longitudeDeg);
      double degAway = Map.GetAngleDistInDegFromHomeTangent(latitudeDeg, longitudeDeg);
      points.push_back(
      {
        (t - _startJulian) * _hScale,
        _vOffset + degAway * _vScale,
        0,
        0.5f, 0.5f, 0.25f, 1.0f
      });
    }
    _solarLine.SetPoints(points);
    
    // Calculate sunrise and sunset
    _sunriseJulian = 0;
    _sunsetJulian = 0;
    for (double t = _startJulian; t <= _endJulian; t += (1.0 / 1440.0))
    {
      double latitudeDeg, longitudeDeg;
      AstronomyService.GetSolarPoint(t, latitudeDeg, longitudeDeg);
      double degAway = Map.GetAngleDistInDegFromHomeTangent(latitudeDeg, longitudeDeg);
      
      if (degAway < 90.0 && _sunriseJulian == 0)
        _sunriseJulian = t;
      if (degAway > 90.0 && _sunriseJulian != 0 && _sunsetJulian == 0)
        _sunsetJulian = t;
    }
    
  }
    
  if (nowJulian > _endJulianMoon || nowJulian < _startJulianMoon)
  {
    // Get the required julian dates
    auto start = Map.GetMapTimeAsLocaltime();
    start.tm_sec = 0;
    start.tm_min = 0;
    start.tm_hour = 12;
    _startJulianMoon = Map.GetJulianDateFromLocaltime(start);
    _endJulianMoon = _startJulianMoon+1.0;
    
    // Create the moon polyline
    std::vector<PolyLinePoint> points; 
    for (double t = _startJulianMoon; t <= (_endJulianMoon+0.05); t += (4.0/(double)Map.width))
    {
      double latitudeDeg, longitudeDeg;
      AstronomyService.GetLunarPoint(t, latitudeDeg, longitudeDeg);
      points.push_back(
      {
        (t - _startJulianMoon) * _hScale,
        _vOffset + Map.GetAngleDistInDegFromHomeTangent(latitudeDeg, longitudeDeg) * _vScale,
        0,
        0.25f, 0.25f, 0.5f, 1.0f
      });
    }
    _lunarLine.SetPoints(points);
  }
  
  // Position the sun and moon
  double sunLatDeg, sunlonDeg, moonLatDeg, moonlonDeg;
  AstronomyService.GetSolarPoint(Map.GetMapTimeAsJulianDate(), sunLatDeg, sunlonDeg);
  AstronomyService.GetLunarPoint(Map.GetMapTimeAsJulianDate(), moonLatDeg, moonlonDeg);
  _sunCircle.SetLocation( (float) ( Map.GetMapTimeAsJulianDate() -  _startJulian) * _hScale,
                           (float) _vOffset + Map.GetAngleDistInDegFromHomeTangent(sunLatDeg, sunlonDeg) * _vScale);    
  _moonCircle.SetLocation( (float) ( Map.GetMapTimeAsJulianDate() -  _startJulianMoon) * _hScale,
                           (float) _vOffset + Map.GetAngleDistInDegFromHomeTangent(moonLatDeg, moonlonDeg) * _vScale);
                           
  // Style the sun and moon
  double sunAngle = Map.GetAngleDistInDegFromHomeTangent(sunLatDeg, sunlonDeg);
  _moonCircle.SetThickness(interpolate(1.5f, 2.5f, invInterp(85, 95, sunAngle)));
  _moonCircle.SetColor(interpolate(_moonColorDay, _moonColorNight, invInterp(85, 95, sunAngle)));
  _sunCircle.SetThickness(interpolate(3.0f, 1.0f, invInterp(85, 95, sunAngle)));
  _sunCircle.SetColor(interpolate(_sunColorDay, _sunColorNight, invInterp(85, 95, sunAngle)));
  
  // Setup the sunrise and sunset labels
  char formatStr[256];
  
  auto sunriseLabelTime = Map.GetLocaltimeFromJulianDate(_sunriseJulian);
  std::strftime(formatStr, 255, "%H:%M", &sunriseLabelTime );
  _sunriseLabel.SetText(formatStr);
  _sunriseLabel.SetFontStyle(FontStyle::Narrow);
  _sunriseLabel.SetColor(0.5,0.375,0.0f,1.0f);
  _sunriseLabel.SetPosition(round((_sunriseJulian - _startJulian) * _hScale), 80);
  _sunriseLabel.SetAlignment(TextAlignment::Center);
  
  auto sunsetLabelTime = Map.GetLocaltimeFromJulianDate(_sunsetJulian);
  std::strftime(formatStr, 255, "%H:%M", &sunsetLabelTime );
  _sunsetLabel.SetText(formatStr);
  _sunsetLabel.SetFontStyle(FontStyle::Narrow);
  _sunsetLabel.SetColor(0.5,0.375,0.0f,1.0f);
  _sunsetLabel.SetPosition(round((_sunsetJulian - _startJulian) * _hScale), 80);
  _sunsetLabel.SetAlignment(TextAlignment::Center);
  
  _tempLabel.SetText(_noaaTemp);
  _tempLabel.SetFontStyle(FontStyle::Regular);
  _tempLabel.SetColor(0.5,0.5,0.5f,1.0f);
  _tempLabel.SetPosition(4, 4);
}

void SolarScene::drawOverride()
{
  _sunriseLabel.Draw();
  _sunsetLabel.Draw();
  _tempLabel.Draw();

  _horizonLine.Draw();
  _lunarLine.Draw();
  _solarLine.Draw();
  
  _moonCircle.Draw();
  _moonCircle.Move(-Map.width, 0);
  _moonCircle.Draw();
  _moonCircle.Move(2.0f * Map.width, 0);
  _moonCircle.Draw();
  
  _sunCircle.Draw();
  _sunCircle.Move(-Map.width, 0);
  _sunCircle.Draw();
  _sunCircle.Move(2.0f * Map.width, 0);
  _sunCircle.Draw();
}
