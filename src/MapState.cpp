#include "MapState.hpp"

#include <math.h>
#include <filesystem>
#include <iostream>
#include <iomanip>
#include <fstream>

using json = nlohmann::json;

#ifdef PI_HOST
static const std::string CONFIG_PATH = "/boot/MantleMapConfig.json";
static const std::string SCENES_PATH = "/home/pi/MantleMap/scenes";
static const std::string EPHEMERIDES_PATH = "/home/pi/MantleMap/de430/linux_p1550p2650.430";
#else
static const std::string CONFIG_PATH = "MantleMapConfig.json";
static const std::string SCENES_PATH = "scenes";
static const std::string EPHEMERIDES_PATH = "linux_p1550p2650.430";
#endif

static double getJulianFromUnix( double unixSecs )
{
  return (unixSecs / 86400.0 ) + 2440587.5;
}

static double getUnixFromJulian( double julian )
{
  return (julian - 2440587.5) * 86400.0;
}


MapState::MapState()
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
  
  // System settings
  sceneResourcePath = GetConfigValue("sceneResourcePath", SCENES_PATH);
  ephemeridesPath = GetConfigValue("ephemeridesPath", EPHEMERIDES_PATH);
  defaultScene = GetConfigValue("defaultScene", std::string("Solar"));
  
  // Don't overwrite a misread file, but feel free to overwrite
  // a missing or correctly read file
  //SaveConfig();
    
  // Internal vars
  AstronomyService.Init(ephemeridesPath);
  _timeMultiplier = 1.0;
  _timePaused = false;
  _isSleeping = false;
  
  ResetTime();
}

MapState::~MapState()
{
  
}

void MapState::SaveConfig()
{
  if (_settingsReadOK) writeConfig();
}

std::string MapState::GetResourcePath(std::string resourceName)
{
  auto filePath = std::filesystem::path(sceneResourcePath) / "Shared" / resourceName;
  if (std::filesystem::exists(filePath))
  {
    return filePath;
  }
  return std::string();
}

bool MapState::readConfig()
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

void MapState::writeConfig()
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
    
void MapState::RunTime()
{
  // Set _referencePoint to now
  // Adjust _mapTimeOffset so maptime doesn't change
  // mark the stopped boolean to false
  
  timepoint_t currMapTime = GetMapTime();
  _referencePoint = std::chrono::system_clock::now();
  _mapTimeOffset =  currMapTime - _referencePoint;
  _timePaused = false;
}

void MapState::PauseTime()
{
  // Set _referencePoint to now
  // Adjust _mapTimeOffset so maptime doesn't change
  // mark the stopped boolean true
  
  timepoint_t currMapTime = GetMapTime();
  _referencePoint = std::chrono::system_clock::now();
  _mapTimeOffset =  currMapTime - _referencePoint;
  _timePaused = true;
}

void MapState::ResetTime()
{
  // Set _mapTimeOffset = 0 and _referencePoint = now
  _timeMultiplier = 1.0;
  _mapTimeOffset = fractionalDays();
  _referencePoint = std::chrono::system_clock::now();
}

void MapState::GoToTime(std::tm& localTime)
{
  // Convert localTime structure to time_t
  time_t localTime_t = mktime(&localTime);
  
  // Convert the time_t to a time point
  timepoint_t localTime_tp = std::chrono::system_clock::from_time_t(localTime_t);

  // Set referencePoint to now and _mapTimeOffset to the difference between the points
  _referencePoint = std::chrono::system_clock::now();
  _mapTimeOffset = localTime_tp - _referencePoint;
}

void MapState::SetSleep(bool value)
{
  _isSleeping = value;
}

bool MapState::GetSleep()
{
  return _isSleeping;
}

void MapState::GoToTimeRelative(fractionalDays offset)
{
  _mapTimeOffset += offset;
}

void MapState::GoToTimeRelative(double offsetInDays)
{
  GoToTimeRelative(fractionalDays(offsetInDays));
}

void MapState::SetTimeMultiplier(double timeMultiplier)
{
  timepoint_t currMapTime = GetMapTime();
  _referencePoint = std::chrono::system_clock::now();
  _mapTimeOffset =  currMapTime - _referencePoint;
  _timeMultiplier = timeMultiplier;
}

double MapState::GetTimeMultiplier()
{
  return _timeMultiplier;
}

timepoint_t MapState::GetMapTime()
{
  // Map time is the difference between reference time and now, 
  // multiplied by the time multiplier,
  // and added to the mapReferenceTime
  
  if (_timePaused)
  {
    return _referencePoint + _mapTimeOffset;
  }
  else
  {
    timepoint_t now = std::chrono::system_clock::now();
    return _referencePoint + _mapTimeOffset + ((now - _referencePoint) * _timeMultiplier);
  }
}

double MapState::GetTimeAsJulianDate()
{
  timepoint_t now = std::chrono::system_clock::now();
  return getJulianFromUnix(std::chrono::duration_cast<fractionalSeconds>(now.time_since_epoch()).count());
}

double MapState::GetMapTimeAsJulianDate()
{
  return getJulianFromUnix(std::chrono::duration_cast<fractionalSeconds>(GetMapTime().time_since_epoch()).count());
}

double MapState::GetJulianDateFromLocaltime(std::tm& localTime)
{
  // Convert localTime structure to time_t
  time_t localTime_t = mktime(&localTime);
  
  // Convert the time_t to a time point
  timepoint_t localTime_tp = std::chrono::system_clock::from_time_t(localTime_t);

  return getJulianFromUnix(std::chrono::duration_cast<fractionalSeconds>(localTime_tp.time_since_epoch()).count());
}

std::tm MapState::GetLocaltimeFromJulianDate(double julian)
{
  double unixDouble = getUnixFromJulian(julian);
  time_t unixTimeT = unixDouble;
  return *localtime(&unixTimeT);
}

std::tm MapState::GetMapTimeAsLocaltime()
{
  timepoint_t mapTime = GetMapTime(); //<class ToDuration, class Clock, class Duration>
  sys_seconds tp = std::chrono::time_point_cast<std::chrono::seconds>(mapTime);
  std::time_t currTime_t = std::chrono::system_clock::to_time_t( tp );
  return *localtime(&currTime_t);
}

std::tm MapState::GetLocaltime()
{
  const auto currTime = std::chrono::system_clock::now();
  std::time_t currTime_t = std::chrono::system_clock::to_time_t( currTime );
  return *localtime(&currTime_t);
}

static void normalizeAngleDegrees(double& angle)
{
  while (angle < -180.0)
    angle += 360.0;
  while (angle > 180.0)
    angle -= 360.0;
}

float MapState::GetAngleDistInDegFromHomeTangent(double latDeg, double lonDeg)
{
  // convert to radians
  double homeLat = homeLatitudeDeg * (M_PI / 180.0);
  double homeLon = homeLongitudeDeg * (M_PI / 180.0);
  double lat = latDeg * (M_PI / 180.0);
  double lon = lonDeg * (M_PI / 180.0);
  
  // Generate the home and sun vectors
  double homeX = cos(homeLon)*cos(homeLat);
  double homeY = sin(homeLon)*cos(homeLat);
  double homeZ = sin(homeLat);
  double objX = cos(lon)*cos(lat);
  double objY = sin(lon)*cos(lat);
  double objZ = sin(lat);
       
  // Calculate the sun angle                
  double sunAngle = acos(homeX * objX + homeY * objY + homeZ * objZ);
  double sunAngleDeg = sunAngle * (180.0 / M_PI);
  normalizeAngleDegrees(sunAngleDeg);
  return abs(sunAngleDeg);
}

float MapState::GetLightBoost(double sunLatDeg, double sunLonDeg)
{
  if (!lightAdjustEnabled)
    return 0.0f;
    
  // convert to radians
  double homeLat = homeLatitudeDeg * (M_PI / 180.0);
  double homeLon = homeLongitudeDeg * (M_PI / 180.0);
  double sunLat = sunLatDeg * (M_PI / 180.0);
  double sunLon = sunLonDeg * (M_PI / 180.0);
  double sunProp = 70 * (M_PI / 180.0);
  double sunFade = 20 * (M_PI / 180.0);
  
  // Generate the home and sun vectors
  double homeX = cos(homeLon)*cos(homeLat);
  double homeY = sin(homeLon)*cos(homeLat);
  double homeZ = sin(homeLat);
  double sunX = cos(sunLon)*cos(sunLat);
  double sunY = sin(sunLon)*cos(sunLat);
  double sunZ = sin(sunLat);
       
  // Calculate the sun angle                
  double sunAngle = acos(homeX * sunX + homeY * sunY + homeZ * sunZ);
  
  if (sunAngle < sunProp)
    return 1.0f;
  else if (sunAngle < (sunProp + sunFade))
    return 1.0f - ((sunAngle - sunProp) / sunFade);

  return 0.0f;
}

