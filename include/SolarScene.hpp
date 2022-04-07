#ifndef SolarScene_HPP
#define SolarScene_HPP

#include "PolyLine.hpp"
#include "TextLabel.hpp"
#include "Scene.hpp"
#include "ConfigService.hpp"
#include "AstronomyService.hpp"

class SolarScene : public Scene
{
 public:
    SolarScene(ConfigService& map, AstronomyService& astro);
    ~SolarScene();
    
    const char* SceneName() override;
    const char* SceneResourceDir() override;

protected:
    void updateOverride() override;
    void drawOverride() override;
    
private:
    AstronomyService& _astro;
    Color _moonColorDay;
    Color _moonColorNight;
    Color _sunColorDay;
    Color _sunColorNight;
    Color _skyColorDay;
    Color _skyColorNight;
    
    PolyLine _solarLine;
    PolyLine _lunarLine;
    PolyLine _horizonLine;
    PolyLine _sunCircle;
    PolyLine _moonCircle;
    
    TextLabel _sunriseLabel;
    TextLabel _sunsetLabel;
    
    double _hScale;
    double _vScale;
    double _vOffset;
    
    double _startJulian;
    double _endJulian;
    double _startJulianMoon;
    double _endJulianMoon;
    
    double _sunriseJulian;
    double _sunsetJulian;
    
    bool _showMoon;
};

#endif
