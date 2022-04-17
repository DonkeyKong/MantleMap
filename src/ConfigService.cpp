#include "ConfigService.hpp"
#include "Utils.hpp"

#include <math.h>
#include <filesystem>
#include <iostream>
#include <iomanip>
#include <fstream>

using json = nlohmann::json;

#ifdef PI_HOST
static const std::string CONFIG_PATH = "/boot/MantleMapConfig.json";
#else
static const std::string CONFIG_PATH = "MantleMapConfig.json";
#endif
static const std::string DEFAULT_SCENES_PATH = "scenes";
static const std::string DEFAULT_EPHEMERIDES_PATH = "linux_p1550p2650.430";

const std::string ConfigService::AllSettings = "*";
ConfigService ConfigService::global;

ConfigService::ConfigService()
{
    _initDone = false;
}

ConfigService::~ConfigService()
{
  
}

void ConfigService::Init()
{
    if (!_initDone)
    {
        _settingsReadOK = readConfig();

        sceneResourcePath_ = GetConfigValue("sceneResourcePath", DEFAULT_SCENES_PATH);
        ephemeridesPath_ = GetConfigValue("ephemeridesPath", DEFAULT_EPHEMERIDES_PATH);
        width_ = GetConfigValue("width", 192);
        height_ = GetConfigValue("height", 96);
        homeLatitudeDeg_ = GetConfigValue("homeLatitudeDeg", 0.0);
        homeLongitudeDeg_ = GetConfigValue("homeLongitudeDeg", 0.0);

        SaveConfig();
        _initDone = true;
    }

    OnSettingChanged(AllSettings);
}

int ConfigService::width() const
{
    return width_;
}

int ConfigService::height() const
{
    return height_;
}

double ConfigService::homeLatitudeDeg() const
{
    return homeLatitudeDeg_;
}

double ConfigService::homeLongitudeDeg() const
{
    return homeLongitudeDeg_;
}

std::string ConfigService::sceneResourcePath() const
{
    return sceneResourcePath_;
}

std::string ConfigService::ephemeridesPath() const
{
    return ephemeridesPath_;
}
const json& ConfigService::GetConfigJson() const
{
    return _config;
}

template <>
void ConfigService::SetConfigValue(const std::string& key, const json& value)
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

void ConfigService::SaveConfig()
{
  if (_settingsReadOK) writeConfig();
}

std::string ConfigService::GetSharedResourcePath(const std::string& resourceName) const
{
  auto filePath = std::filesystem::path(sceneResourcePath_) / "Shared" / resourceName;
  if (std::filesystem::exists(filePath))
  {
    return filePath;
  }
  return std::string();
}

bool ConfigService::readConfig()
{
  // Clear the existing config and set it up as a json object
  _config = json::object();
  
  // If the config file doesn't exist, then we are done. 
  // Consider it "read" so we can overwrite the file.
  if (!std::filesystem::exists(CONFIG_PATH))
  {
    std::cout << "Config file missing, using defaults." << std::endl;
    return true;
  }
  
  // Try to read the file
  try
  {
    std::ifstream ifs(CONFIG_PATH);
    _config = json::parse(ifs);
    ifs.close();
    std::cout << "Read and parsed config file!" << std::endl;
  }
  catch (...)
  {
    std::cout << "Failed to read or parse config file!" << std::endl;
    std::cout << "Delete it of fix permissions to generate a new one." << std::endl;
    return false;
  }
  
  // If the config is some nonsense, clear it
  if (!_config.is_object()) 
  {
    std::cout << "Config was parsed but invalid!" << std::endl;
    std::cout << "Delete it to generate a new one." << std::endl;
    _config = json::object();
    return false;
  }	

  return true;
}

void ConfigService::writeConfig()
{
  try
  {
    std::ofstream ofs(CONFIG_PATH, std::ofstream::out | std::ofstream::trunc);
    ofs << std::setw(4) << _config;
    ofs.flush();
    ofs.close();
    
    if ( (ofs.rdstate() & std::ifstream::failbit ) != 0 )
    {
      std::cout << "Failed to write config file!" << std::endl;
    }
    else
    {
      std::cout << "Wrote config file!" << std::endl;
    }
    //std::cout << _config << std::endl;
  }
  catch (...)
  {
    std::cout << "Failed to write config file!" << std::endl;
  }
}


