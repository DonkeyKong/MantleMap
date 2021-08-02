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

#include "LightScene.hpp"

LightScene::LightScene(MapState& map) : Scene(map, SceneType::Base, SceneLifetime::Manual)
{
  sunCurrentLat = 0;
  sunCurrentLon = 0;
  sunTargetLat = 0;
  sunTargetLon = 0;
  sunPropAngleCurrent = 0;
  overrideSunLocation = false;
  sunPropAngleTarget = Map.sunPropigationDeg;
}

LightScene::~LightScene()
{
}

const char* LightScene::SceneName()
{
  return "Sunlight Map";
}

const char* LightScene::SceneResourceDir()
{
  return "Light";
}

void LightScene::initGLOverride()
{
  // Create and setup mapLayer1Texture
  mapLayer1Texture = loadImageToTexture(mapImagePath.c_str());
  
  // Create and setup mapLayer2Texture
  mapLayer2Texture = loadImageToTexture(mapImageNightPath.c_str());
  
  // Load and compile the shaders into a glsl program
  program = loadGraphicsProgram(vertShader, fragShader);
  program.SetCameraFromPixelTransform(Map.width,Map.height);
}

bool LightScene::Query(std::string query, std::string& response)
{

  // Use this regex for sun movement queries
  {
    std::smatch match; 
    std::regex sunMoveRegex(R"((?:move|send|place)\s+(?:the sun|sun|sol)\s+(?:to|at)?\s*(-?[0-9]+(?:\.)?[0-9]*)\s*(?:degrees|degree|deg|º)?\s*(north|south|east|west|n|s|e|w)?(?:\s*,\s*|\s+)(-?[0-9]+(?:\.)?[0-9]*)\s*(?:degrees|degree|deg|º)?\s*(north|south|east|west|n|s|e|w)?)", std::regex_constants::icase);
    if (std::regex_match(query, match, sunMoveRegex))
    {
      // These two doubles are what we are looking to extract
      double latDeg = 0;
      double lonDeg = 0;
    
      // Captured groups are:
      // 1 - coord one
      // 2 - coord direction (assume North if not specified)
      // 3 - coord two
      // 4 - coord direction (assume East if not specified)
      switch(match[2].str()[0])
      {
        case '\0': case 'n': case 'N':
          latDeg = std::stod(match[1]);
          break;
        case 's': case 'S':
          latDeg = -std::stod(match[1]);
          break;
        case 'e': case 'E':
          lonDeg = std::stod(match[1]);
          break;
        case 'w': case 'W':
          lonDeg = -std::stod(match[1]);
          break;
      }
    
      switch(match[4].str()[0])
      {
        case 'n': case 'N':
          latDeg = std::stod(match[3]);
          break;
        case 's': case 'S':
          latDeg = -std::stod(match[3]);
          break;
        case '\0': case 'e': case 'E':
          lonDeg = std::stod(match[3]);
          break;
        case 'w': case 'W':
          lonDeg = -std::stod(match[3]);
          break;
      }
    
      std::stringstream output;
      output << "Moving the sun to " 
             << abs(latDeg) << "º " << (latDeg>=0?"North":"South") << " " 
             << abs(lonDeg) << "º " << (lonDeg>=0?"East":"West");
      response = output.str();
    
      overrideSunLocation = true;
      sunTargetLat = latDeg;
      sunTargetLon = lonDeg;
    
      return true;
    }
  }
  
  // Use this regex for resetting sun movement queries
  {
    if (query == "reset sun location")
    {
      response = "Sun location set by time";
    
      overrideSunLocation = false;
      sunTargetLat = 0;
      sunTargetLon = 0;
    
      return true;
    }
  }
  
  // Use this regex for delta time queries
  {
    std::smatch match; 
    std::regex deltaTimeRegex(R"((?:move|go|travel|show me)\s+(forwards?|backwards?|back)?(\s+(?:and\s+)?(-?[0-9]+(?:\.)?[0-9]*)\s+(minutes?|hours?|days?|weeks?|months?))+)", std::regex_constants::icase);
    if (std::regex_match(query, match, deltaTimeRegex))
    {
      double dir = 1;
      double delta = 0;
      // Captured groups are:
      // 1 - backward
      // 2 - and 38 minutes
      // 3 - 38
      // 4 - minutes
      // ...(2,3,4 repeating)
      
     if (match[1].str()[0] == 'b')
        dir = -1;
      
      for (int i=3; i < (int)match.size(); i+=3)
      {
        if (match[i+1].str().rfind("mi", 0) == 0)
        {
          delta += std::stod(match[i]) / 24.0 / 60.0;
        }
        else if (match[i+1].str().rfind("h", 0) == 0)
        {
          delta += std::stod(match[i]) / 24.0;
        }
        else if (match[i+1].str().rfind("d", 0) == 0)
        {
          delta += std::stod(match[i]);
        }
        else if (match[i+1].str().rfind("w", 0) == 0)
        {
          delta += std::stod(match[i]) * 7.0;
        }
        else if (match[i+1].str().rfind("mo", 0) == 0)
        {
          delta += round(std::stod(match[i]) * (365.0 / 12.0));
        }
      }
      
      Map.GoToTimeRelative(delta * dir);

      std::stringstream output;
      output << "Time adjustment adjusted by " << delta * dir << " days" << std::endl;
      response = output.str();
    
      return true;
    }
  }
  
  // Use this regex for time multiplier queries
  {
    std::smatch match; 
    std::regex deltaTimeRegex(R"(set\s+(?:time speed|time|speed)\s+multiplier\s+(?:to\s+)?(-?[0-9]+(?:\.)?[0-9]*))", std::regex_constants::icase);
    if (std::regex_match(query, match, deltaTimeRegex))
    {
      double timeMultiplier = 1.0;
     
      timeMultiplier = std::stod(match[1]);
      
      Map.SetTimeMultiplier(timeMultiplier);
      
      std::stringstream output;
      output << "Speed multiplier set to " << timeMultiplier << " days" << std::endl;
      response = output.str();
    
      return true;
    }
  }
  
  // Use this regex for day / night requests
  {
    std::smatch match; 
    std::regex deltaTimeRegex(R"(show\s+(day|night|lightmap|light)(?:\s+map)?)", std::regex_constants::icase);
    if (std::regex_match(query, match, deltaTimeRegex))
    {
      if (match[1].str()[0] == 'd')
      {
        sunPropAngleTarget = 180;
        std::stringstream output;
        output << "Showing day map" << std::endl;
        response = output.str();
      }
      else if (match[1].str()[0] == 'n')
      {
        sunPropAngleTarget = 0;
        std::stringstream output;
        output << "Showing night map" << std::endl;
        response = output.str();
      }
      else if (match[1].str()[0] == 'l')
      {
        sunPropAngleTarget = Map.sunPropigationDeg;
        std::stringstream output;
        output << "Showing light map" << std::endl;
        response = output.str();
      }
    
      return true;
    }
  }
  
  // Use this regex for light propigation queries
  {
    std::smatch match; 
    std::regex lightPropigationRegex(R"((?:set)\s+(?:light propigation)\s+(?:to)?\s*(-?[0-9]+(?:\.)?[0-9]*)\s*(?:degrees|degree|deg|º)?)", std::regex_constants::icase);
    if (std::regex_match(query, match, lightPropigationRegex))
    {
      // This double is our only real variable
      double lightPropigationDeg = std::stod(match[1]);
    
      std::stringstream output;
      output << "Setting light propigation to " << lightPropigationDeg << "º ";
      response = output.str();
    
      
      sunPropAngleTarget = lightPropigationDeg;
    
      return true;
    }
  }
  
  return false;
}

void LightScene::resetOverride(bool animate)
{
  overrideSunLocation = false;
  sunPropAngleTarget = Map.sunPropigationDeg;
  Update();
  
  // If we want to skip animations, force things instantly into the right place
  if (!animate)
  {
    sunCurrentLat = sunTargetLat;
    sunCurrentLon = sunTargetLon;
    sunPropAngleCurrent = sunPropAngleTarget;
  }
}

void LightScene::updateOverride()
{
  // Update the sun's location
  if (!overrideSunLocation)
  {
    Map.AstronomyService.GetSolarPoint(Map.GetMapTimeAsJulianDate(), sunTargetLat, sunTargetLon);
  }
  
  moveTowardsAngleDeg2D(sunCurrentLat, sunCurrentLon, sunTargetLat, sunTargetLon, 0.5 * Map.GetTimeMultiplier());
  
  moveTowards(sunPropAngleCurrent, sunPropAngleTarget, 0.5f);
}

void LightScene::drawOverride()
{
	// Select our shader program
	glUseProgram(program.GetId());
	
	// Bind the day, night, and lon lat lookup textures to units 0, 1, and 2
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, mapLayer1Texture);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, mapLayer2Texture);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, LonLatLookupTexture);
  
  // Tell our shader which units to look for each texture on
  program.SetUniform("uTexture1", 0);
  program.SetUniform("uTexture2", 1);
  program.SetUniform("uLonLatLut", 2);
  program.SetUniform("uSunPropigationRad", sunPropAngleCurrent * (float)(M_PI / 180.0));
  
  // Tell the frag shader the size of the map in pixels
  program.SetUniform("uScale", Map.width, Map.height);
  
  {
    double lat, lon;
    Map.AstronomyService.GetSolarPoint(Map.GetTimeAsJulianDate(), lat, lon);
    program.SetUniform("uLightBoost", Map.GetLightBoost(lat, lon));
  }
  
  
  program.SetUniform("uDrawSun", true );
  program.SetUniform("uDrawMoon", true );
  
  // Send the sun's current location to the shader program
  program.SetUniform("uSunLonLat", (float)(sunCurrentLon * (M_PI / 180.0)), (float)(sunCurrentLat * (M_PI / 180.0)));
  
  // Do the same for the moon
  double lat, lon;
  Map.AstronomyService.GetLunarPoint(Map.GetMapTimeAsJulianDate(), lat, lon);
  program.SetUniform("uMoonLonLat", (float)(lon * (M_PI / 180.0)), (float)(lat * (M_PI / 180.0)));
  
  // Draw a full map-sized rectagle using the current shader
	drawMapRect();
}
