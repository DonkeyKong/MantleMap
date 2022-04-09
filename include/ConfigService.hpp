#pragma once

#include <chrono>
#include <string>
#include <ctime>
#include <iostream>
#include <nlohmann/json.hpp>

class ConfigService
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
    int fpsLimit;
    
    // Constructor
    ConfigService();
    ~ConfigService();
    
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
    bool _settingsReadOK;
};

