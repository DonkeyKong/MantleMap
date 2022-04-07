#include "SolarScene.hpp"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

SolarScene::SolarScene(ConfigService& map, AstronomyService& astro) : Scene(map, SceneType::Base, SceneLifetime::Manual),
  _astro(astro),
  _solarLine(map), _horizonLine(map), _sunCircle(map), _lunarLine(map), _moonCircle(map),
  _sunriseLabel(map), _sunsetLabel(map)
{   
  _showMoon = map.GetConfigValue("solarShowMoon", true);
  
  _vScale = (double)map.height / 180.0 * 0.9;
  _vOffset = (double)map.height * 0.05;
  _hScale = (double)map.width;
  
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
  
  _horizonLine.AddPoint({ 0, (float)_vOffset + 90.0f * (float)_vScale, 0,
                          {0.2f, 0.2f, 0.2f, 1.0f}});
  _horizonLine.AddPoint({ 1.0f * (float)_hScale, (float)_vOffset + 90.0f * (float)_vScale, 0,
    {0.2f, 0.2f, 0.2f, 1.0f}});
  _horizonLine.SetThickness(2.0f);
    
  float sunRadius = 5.0f;
  float moonRadius = 3.0f;
  for (float a = 0; a <= (float)(2.1*M_PI); a+= 0.1f)
  {
    _sunCircle.AddPoint({ sunRadius * sin(a), sunRadius * cos(a), 0,
    {1, 1, 1, 1}});
    _moonCircle.AddPoint({ moonRadius * sin(a), moonRadius * cos(a), 0,
    {1, 1, 1, 1}});
  }
  
  _lunarLine.SetThickness(1.5f);
  _solarLine.SetThickness(2.0f);
}

SolarScene::~SolarScene()
{ 
}

const char* SolarScene::SceneName()
{
  return "Solar Graph";
}

const char* SolarScene::SceneResourceDir()
{
  return "Solar";
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
  double nowJulian = TimeService::GetSceneTimeAsJulianDate();
  
  if (nowJulian > _endJulian || nowJulian < _startJulian)
  {
    // Get the required julian dates
    auto start = TimeService::GetLocaltimeFromJulianDate(nowJulian);
    start.tm_sec = 0;
    start.tm_min = 0;
    start.tm_hour = 0;
    _startJulian = TimeService::GetJulianDateFromLocaltime(start);
    _endJulian = _startJulian+1.0;

    // Create the sun polyline
    std::vector<Vertex> points;
    double sunriseJulianGuess = 0;
    double sunsetJulianGuess = 0;
    for (double t = _startJulian; t <= (_endJulian+0.05); t += (4.0/(double)config.width))
    {
      double latitudeDeg, longitudeDeg;
      _astro.GetSolarPoint(t, latitudeDeg, longitudeDeg);
      double degAway = _astro.GetAngleDistInDegFromHomeTangent(latitudeDeg, longitudeDeg);
      points.push_back(
      {
        {
          (float)((t - _startJulian) * _hScale),
          (float)(_vOffset + degAway * _vScale),
          0,
        },
        {0.5f, 0.5f, 0.25f, 1.0f}
      });
      
      if (degAway < 90.0 && sunriseJulianGuess == 0)
        sunriseJulianGuess = t;
      if (degAway > 90.0 && sunriseJulianGuess != 0 && sunsetJulianGuess == 0)
        sunsetJulianGuess = t;
    }
    _solarLine.SetPoints(points);
    
    _sunriseJulian = 0;
    _sunsetJulian = 0;
    if (sunriseJulianGuess != 0 && sunsetJulianGuess != 0)
    {
      // Calculate sunrise
      for (double t = sunriseJulianGuess; t > _startJulian; t -= (1.0 / 1440.0))
      {
        double latitudeDeg, longitudeDeg;
        _astro.GetSolarPoint(t, latitudeDeg, longitudeDeg);
        double degAway = _astro.GetAngleDistInDegFromHomeTangent(latitudeDeg, longitudeDeg);
        if (degAway >= 90.0)
        {
          _sunriseJulian = t;
          break;
        }
      }
    
      // Calculate sunset
      for (double t = sunsetJulianGuess; t > _startJulian; t -= (1.0 / 1440.0))
      {
        double latitudeDeg, longitudeDeg;
        _astro.GetSolarPoint(t, latitudeDeg, longitudeDeg);
        double degAway = _astro.GetAngleDistInDegFromHomeTangent(latitudeDeg, longitudeDeg);
        if (degAway <= 90.0)
        {
          _sunsetJulian = t;
          break;
        }
      }
    }
  }
    
  if (nowJulian > _endJulianMoon || nowJulian < _startJulianMoon)
  {
    // Get the required julian dates
    auto start = TimeService::GetLocaltimeFromJulianDate(nowJulian);
    start.tm_sec = 0;
    start.tm_min = 0;
    start.tm_hour = 12;
    _startJulianMoon = TimeService::GetJulianDateFromLocaltime(start);
    
    if (nowJulian < _startJulianMoon)
      _startJulianMoon -= 1.0;
    
    _endJulianMoon = _startJulianMoon+1.0;
    
    // Create the moon polyline
    std::vector<Vertex> points; 
    for (double t = _startJulianMoon; t <= (_endJulianMoon+0.05); t += (4.0/(double)config.width))
    {
      double latitudeDeg, longitudeDeg;
      _astro.GetLunarPoint(t, latitudeDeg, longitudeDeg);
      points.push_back(
      {
        {
          (float)((t - _startJulianMoon) * _hScale),
          (float)(_vOffset + _astro.GetAngleDistInDegFromHomeTangent(latitudeDeg, longitudeDeg) * _vScale),
          0
        },
        {0.25f, 0.25f, 0.5f, 1.0f}
      });
    }
    _lunarLine.SetPoints(points);
  }
  
  // Position the sun and moon
  double sunLatDeg, sunlonDeg, moonLatDeg, moonlonDeg;
  _astro.GetSolarPoint(TimeService::GetSceneTimeAsJulianDate(), sunLatDeg, sunlonDeg);
  _astro.GetLunarPoint(TimeService::GetSceneTimeAsJulianDate(), moonLatDeg, moonlonDeg);
  _sunCircle.SetLocation( (float) ( TimeService::GetSceneTimeAsJulianDate() -  _startJulian) * _hScale,
                           (float) _vOffset + _astro.GetAngleDistInDegFromHomeTangent(sunLatDeg, sunlonDeg) * _vScale);    
  _moonCircle.SetLocation( (float) ( TimeService::GetSceneTimeAsJulianDate() -  _startJulianMoon) * _hScale,
                           (float) _vOffset + _astro.GetAngleDistInDegFromHomeTangent(moonLatDeg, moonlonDeg) * _vScale);
                           
  // Style the sun and moon
  double sunAngle = _astro.GetAngleDistInDegFromHomeTangent(sunLatDeg, sunlonDeg);
  _moonCircle.SetThickness(interpolate(1.5f, 2.5f, invInterp(85, 95, sunAngle)));
  _moonCircle.SetColor(interpolate(_moonColorDay, _moonColorNight, invInterp(85, 95, sunAngle)));
  _sunCircle.SetThickness(interpolate(3.0f, 1.0f, invInterp(85, 95, sunAngle)));
  _sunCircle.SetColor(interpolate(_sunColorDay, _sunColorNight, invInterp(85, 95, sunAngle)));
  
  // Setup the sunrise and sunset labels
  char formatStr[256];
  
  if (_sunriseJulian != 0) 
  {
    auto sunriseLabelTime = TimeService::GetLocaltimeFromJulianDate(_sunriseJulian);
    std::strftime(formatStr, 255, "%H:%M", &sunriseLabelTime );
    _sunriseLabel.SetText(formatStr);
    _sunriseLabel.SetFontStyle(FontStyle::Narrow);
    _sunriseLabel.SetColor(0.5,0.375,0.0f,1.0f);
    _sunriseLabel.SetPosition( MIN(round((_sunriseJulian - _startJulian) * _hScale), 50), 80);
    _sunriseLabel.SetAlignment(HAlign::Center);
  }
  
  if (_sunsetJulian != 0) 
  {
    auto sunsetLabelTime = TimeService::GetLocaltimeFromJulianDate(_sunsetJulian);
    std::strftime(formatStr, 255, "%H:%M", &sunsetLabelTime );
    _sunsetLabel.SetText(formatStr);
    _sunsetLabel.SetFontStyle(FontStyle::Narrow);
    _sunsetLabel.SetColor(0.5,0.375,0.0f,1.0f);
    _sunsetLabel.SetPosition(  MAX(round((_sunsetJulian - _startJulian) * _hScale), 142), 
                               80);
    _sunsetLabel.SetAlignment(HAlign::Center);
  }
}

void SolarScene::drawOverride()
{
  if (_sunriseJulian != 0) 
    _sunriseLabel.Draw();
  if (_sunsetJulian != 0) 
  _sunsetLabel.Draw();

  _horizonLine.Draw();
  if (_showMoon) 
  {
    _lunarLine.Draw();
  }	
  _solarLine.Draw();
  
  if (_showMoon) 
  {
    _moonCircle.Draw();
    _moonCircle.Move(-config.width, 0);
    _moonCircle.Draw();
    _moonCircle.Move(2.0f * config.width, 0);
    _moonCircle.Draw();
  }
  
  _sunCircle.Draw();
  _sunCircle.Move(-config.width, 0);
  _sunCircle.Draw();
  _sunCircle.Move(2.0f * config.width, 0);
  _sunCircle.Draw();
}
