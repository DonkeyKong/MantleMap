#ifndef MAPSTATE_HPP
#define MAPSTATE_HPP

#include "Astronomy.hpp"

#include <chrono>
#include <string>
#include <ctime>
#include <iostream>
#include <nlohmann/json.hpp>

typedef std::chrono::duration<double, std::ratio<1, 1>> fractionalSeconds;
typedef std::chrono::duration<double, std::ratio<86400, 1>> fractionalDays;
typedef std::chrono::time_point<std::chrono::system_clock, fractionalDays > timepoint_t;
typedef std::chrono::time_point<std::chrono::system_clock, fractionalSeconds > timepoint_seconds_t;

typedef std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds> sys_seconds;

class MapState
{
  public:
  
    // Public fields (tbd: eliminate or make private)
    int width;
    int height;
    int marginTop;
    int marginBottom;
    int marginLeft;
    int marginRight;
    float latitudeCenterDeg;
    float longitudeCenterDeg;
    float sunPropigationDeg;
    double homeLatitudeDeg;
    double homeLongitudeDeg;
    bool lightAdjustEnabled;
    std::string defaultScene;
    std::string sceneResourcePath;
    std::string ephemeridesPath;
    
    Astronomy AstronomyService;
    
    // Constructor
    MapState();
    ~MapState();
    
    // Control the flow of time
    void RunTime();
    void PauseTime();
    void ResetTime();
    void GoToTime(std::tm& localTime);
    void GoToTimeRelative(fractionalDays offset);
    void GoToTimeRelative(double offsetInDays);
    void SetTimeMultiplier(double timeMultiplier);
    double GetTimeMultiplier();
    
    // Time utility functions
    timepoint_t GetMapTime();
    double GetTimeAsJulianDate();
    double GetMapTimeAsJulianDate();
    std::tm GetMapTimeAsLocaltime();
    std::tm GetLocaltime();
    double GetJulianDateFromLocaltime(std::tm&);
    std::tm GetLocaltimeFromJulianDate(double);
    
    // Light angle utilities
    float GetAngleDistInDegFromHomeTangent(double latDeg, double lonDeg);
    float GetLightBoost(double sunLatDeg, double sunLonDeg);
    
    // Map sleep property
    void SetSleep(bool value);
    bool GetSleep();
    
    // Resource path utils
    std::string GetResourcePath(std::string resourceName);
    
    // Configuration functions
    template <typename T>
    T GetConfigValue(std::string key, T defaultValue)
    {
      try
      {
        if (_config.contains(key))
        {
          defaultValue = _config[key];  // Try to read this value from the config
          //std::cout << "Read " << key << std::endl;
        }
        else
        {
          _config[key] = defaultValue;
          //std::cout << "Setting " << key << std::endl;
        }
      }
      catch (...)
      {
        _config[key] = defaultValue;  // If anything bad happened, write/overwrite with the default
        //std::cout << "Setting " << key << std::endl;
      }
      
      return defaultValue;
    }
    
    template <typename T>
    void SetConfigValue(std::string key, T value)
    {
      try
      {
        _config[key] = value;
      }
      catch (...)
      {
      }
    }
    
    void SaveConfig();
    
  private: 
    // Internal config
    bool readConfig();
    void writeConfig();
    nlohmann::json _config;
    
    // Internal time state vars
    timepoint_t _referencePoint;
    fractionalDays _mapTimeOffset;
    bool _isSleeping;
    double _timeMultiplier;
    bool _timePaused;
    bool _settingsReadOK;
};

#endif /* MAPSTATE_HPP */
