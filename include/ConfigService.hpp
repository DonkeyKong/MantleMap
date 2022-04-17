#pragma once

#include <chrono>
#include <string>
#include <ctime>
#include <iostream>
#include <sigslot/signal.hpp>
#include <nlohmann/json.hpp>

class ConfigService
{
  public:
    // Constructor
    ConfigService();
    ~ConfigService();

    // Const property accessors
    int width() const;
    int height() const;
    double homeLatitudeDeg() const;
    double homeLongitudeDeg() const;
    std::string sceneResourcePath() const;
    std::string ephemeridesPath() const;

    void Init();
    
    // Resource path utils
    std::string GetSharedResourcePath(const std::string& resourceName) const;

    // Configuration functions
    template <typename T>
    T GetConfigValue(const std::string& key, const T& defaultValue)
    {
        T configValue;
      try
      {
        if (_config.contains(key))
        {
          configValue = _config[key];  // Try to read this value from the config
          //std::cout << "Read " << key << std::endl;
        }
        else
        {
          SetConfigValue(key, defaultValue);
          configValue = defaultValue;
          //std::cout << "Setting " << key << std::endl;
        }
      }
      catch (...)
      {
        SetConfigValue(key, defaultValue);
        configValue = defaultValue;
        //std::cout << "Setting " << key << std::endl;
      }
      
      return configValue;
    }
    
    template <typename T>
    void SetConfigValue(const std::string& key, const T& value)
    {
      try
      {
        if (!_config.contains(key) || _config[key] != value)
        {
            _config[key] = value;
            if (_initDone) OnSettingChanged(key);
        }
      }
      catch (...)
      {
      }
    }

    template <typename T>
    bool UpdateIfChanged(T& val, const std::string& eventStr, const std::string& key, const T& defaultValue)
    {
        if (eventStr == key || eventStr == AllSettings)
        {
            val = GetConfigValue(key, defaultValue);
            return true;
        }
        return false;
    }
    
    void SaveConfig();

    bool HasKey(const std::string key)
    {
        return _config.contains(key);
    }

    bool ValueTypeMatches(const std::string& key, const nlohmann::json& value)
    {
        if (!HasKey(key)) return false;
        return _config[key].type() == value.type();
    }

    const nlohmann::json& GetConfigJson() const;

    void Subscribe(const std::function <void (std::string)>& handler) 
    {
        handler(AllSettings);
        OnSettingChanged.connect(handler);
    }
    
    static const std::string AllSettings;

    static ConfigService global;
  private: 
    // Internal config
    bool readConfig();
    void writeConfig();
    nlohmann::json _config;
    bool _settingsReadOK;
    bool _initDone;

    // If the event is raised with ConfigService::AllSettings, that means a full file refresh
    sigslot::signal<std::string> OnSettingChanged;

    // These cannot be changed after boot and must be editied in
    // the config file
    int width_;
    int height_;
    double homeLatitudeDeg_;
    double homeLongitudeDeg_;
    std::string sceneResourcePath_;
    std::string ephemeridesPath_;
};

