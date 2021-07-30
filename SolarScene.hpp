#ifndef SolarScene_HPP
#define SolarScene_HPP

#include "EGL/egl.h"
#include "EGL/eglplatform.h"
#include "GLES2/gl2.h"
#include "EGL/eglext.h"

#include "PolyLine.hpp"
#include "TextLabel.hpp"
#include "Scene.hpp"
#include "MapState.hpp"

class SolarScene : public Scene
{
 public:
    SolarScene(MapState& map);
    ~SolarScene();
    
    const char* SceneName() override;
    const char* SceneResourceDir() override;

protected:
    void initGLOverride() override;
    void updateOverride() override;
    void drawOverride() override;
    
private:
    
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
};

#endif
