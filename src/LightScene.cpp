#include "LightScene.hpp"
#include "AnimationUtil.hpp"
#include "ConfigService.hpp"
static auto& config = ConfigService::global;

#include <regex>
#include <algorithm>
#include <string>
#include <sstream>
#include <chrono>
#include <ctime>

LightScene::LightScene(AstronomyService& astro) : 
  Scene(SceneType::Base, SceneLifetime::Manual),
  astro(astro)
{

        // Sunlight map animation vars
    sunCurrentLat = 0;
    sunCurrentLon = 0;
    sunTargetLat = 0;
    sunTargetLon = 0;
    sunPropAngleCurrent = 0;

    // Sunlight map settings
    config.Subscribe([&](const ConfigUpdateEventArg& arg)
    {
        arg.UpdateIfChanged("scenes.Light.sunPropigationDeg", sunPropigationDeg, 80.0f);
        arg.UpdateIfChanged("scenes.Light.lightAdjustEnabled", lightAdjustEnabled, true);
        arg.UpdateIfChanged("scenes.Light.overrideSunLocation", overrideSunLocation, false);
        arg.UpdateIfChanged("scenes.Light.sunOverrideLat", sunOverrideLat, 0.0);
        arg.UpdateIfChanged("scenes.Light.sunOverrideLon", sunOverrideLon, 0.0);

        if (overrideSunLocation)
        {
            sunTargetLat = sunOverrideLat;
            sunTargetLon = sunOverrideLon;
        }
    });


}

LightScene::~LightScene()
{
}

const char* LightScene::SceneName()
{
  return "Light";
}

void LightScene::initGLOverride()
{
    // Create the LonLatLookupTexture
    ImageRGBA lut = projection.getInvLookupTable();
    LonLatLookupTexture = std::make_unique<GfxTexture>(lut);

    // Create and setup mapLayer1Texture
    mapLayer1Texture = loadTexture("map_day.png");
    
    // Create and setup mapLayer2Texture
    mapLayer2Texture = loadTexture("map_night.png");
    
    // Load and compile the shaders into a glsl program
    program = loadProgram("vertshader.glsl", "lightfragshader.glsl", 
                            {
                                ShaderFeature::PixelSnap,
                                ShaderFeature::Texture
                            });

    // Create the mesh for the image view
    //       X                  Y                          Z       U       V
    mesh = { 0.0f,                0.0f,                   0.0f,   0.0f,   0.0f,
            (float)mapLayer1Texture->GetWidth(), 0.0f,                   0.0f,   1.0f,   0.0f, 
            0.0f,                       (float)mapLayer1Texture->GetHeight(),  0.0f,   0.0f,   1.0f,
            (float)mapLayer1Texture->GetWidth(), (float)mapLayer1Texture->GetHeight(),  0.0f,   1.0f,   1.0f  };
}

void LightScene::resetOverride(bool animate)
{
  overrideSunLocation = false;
  Update();
  
  // If we want to skip animations, force things instantly into the right place
  if (!animate)
  {
    sunCurrentLat = sunTargetLat;
    sunCurrentLon = sunTargetLon;
    sunPropAngleCurrent = sunPropigationDeg;
  }
}

void LightScene::updateOverride()
{
  // Update the sun's location
  if (!overrideSunLocation)
  {
    astro.GetSolarPoint(TimeService::GetSceneTimeAsJulianDate(), sunTargetLat, sunTargetLon);
  }
  
  moveTowardsAngleDeg2D(sunCurrentLat, sunCurrentLon, sunTargetLat, sunTargetLon, 0.5 * TimeService::GetSceneTimeMultiplier());
  
  moveTowards(sunPropAngleCurrent, sunPropigationDeg, 0.5f);
}

void LightScene::drawOverride()
{
	// Select our shader program
    program->Use();
	
	// Bind the day, night, and lon lat lookup textures to units 0, 1, and 2
    program->SetTexture0(*mapLayer1Texture);
    program->SetTexture1(*mapLayer2Texture);
    program->SetTexture2(*LonLatLookupTexture);
    
    // Set some additional uniforms our special shader uses
    program->SetUniform("uSunPropigationRad", sunPropAngleCurrent * (float)(M_PI / 180.0));
    {
        double lat, lon;
        astro.GetSolarPoint(TimeService::GetSceneTimeAsJulianDate(), lat, lon);
        program->SetUniform("uLightBoost", lightAdjustEnabled ? astro.GetLightBoost(lat, lon) : 0.0f);
    }
    
    program->SetUniform("uDrawSun", true );
    program->SetUniform("uDrawMoon", true );
    
    // Send the sun's current location to the shader program
    program->SetUniform("uSunLonLat", (float)(sunCurrentLon * (M_PI / 180.0)), (float)(sunCurrentLat * (M_PI / 180.0)));
    
    // Do the same for the moon
    double lat, lon;
    astro.GetLunarPoint(TimeService::GetSceneTimeAsJulianDate(), lat, lon);
    program->SetUniform("uMoonLonLat", (float)(lon * (M_PI / 180.0)), (float)(lat * (M_PI / 180.0)));
    
    // Finally, setup the main billboard render
    glVertexAttribPointer(
                  program->Attrib("aPosition"),      // The attribute ID
                  3,                  // size
                  GL_FLOAT,           // type
                  GL_FALSE,           // normalized?
                  5*sizeof(float),                  // stride
                  mesh.data()         // underlying data
          );

    glEnableVertexAttribArray ( program->Attrib("aPosition") );
    
    glVertexAttribPointer(
                        program->Attrib("aTexCoord"), // The attribute ID
                        2,                  // size
                        GL_FLOAT,           // type
                        GL_FALSE,           // normalized?
                        5*sizeof(float),   // stride
                        mesh.data()+3      // underlying data
                );
                
    glEnableVertexAttribArray(program->Attrib("aTexCoord"));

    // Draw the triangles!
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
