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
static const std::string DEFAULT_SCENE_NAME = "Solar";


ConfigService::ConfigService()
{

}

ConfigService::~ConfigService()
{
  
}

void ConfigService::Init()
{
    _settingsReadOK = readConfig();

    // LED matrix settings
    width = GetConfigValue("width", 192);
    height = GetConfigValue("height", 96);

    // Sunlight map settings
    marginTop = GetConfigValue("marginTop", 1);
    marginBottom = GetConfigValue("marginBottom", 2);
    marginLeft = GetConfigValue("marginLeft", 7);
    marginRight = GetConfigValue("marginRight", 8);
    latitudeCenterDeg = GetConfigValue("latitudeCenterDeg", 0.0f);
    longitudeCenterDeg = GetConfigValue("longitudeCenterDeg", 156.0f);
    sunPropigationDeg = GetConfigValue("sunPropigationDeg", 80.0f);

    // Home location
    homeLatitudeDeg = GetConfigValue("homeLatitudeDeg", 0.0);
    homeLongitudeDeg = GetConfigValue("homeLongitudeDeg", 0.0);

    // Render settings
    lightAdjustEnabled = GetConfigValue("lightAdjustEnabled", true);
    fpsLimit = GetConfigValue("fpsLimit", 60);

    // System settings
    sceneResourcePath = GetConfigValue("sceneResourcePath", DEFAULT_SCENES_PATH);
    ephemeridesPath = GetConfigValue("ephemeridesPath", DEFAULT_EPHEMERIDES_PATH);
    defaultScene = GetConfigValue("defaultScene", DEFAULT_SCENE_NAME);
}

void ConfigService::SaveConfig()
{
  if (_settingsReadOK) writeConfig();
}

std::string ConfigService::GetResourcePath(std::string resourceName)
{
  auto filePath = std::filesystem::path(sceneResourcePath) / "Shared" / resourceName;
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


