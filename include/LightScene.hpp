#pragma once

#include "Scene.hpp"
#include "AstronomyService.hpp"

class LightScene : public Scene
{
 public:
    LightScene(ConfigService& config, AstronomyService& astro);
    ~LightScene();
    
    const char* SceneName() override;
    const char* SceneResourceDir() override;
    bool Query(std::string query, std::string& response) override;

protected:
    void initGLOverride() override;
    void resetOverride(bool animate) override;
    void updateOverride() override;
    void drawOverride() override;
    
private:
    std::unique_ptr<GfxProgram> program;
    std::unique_ptr<GfxTexture> mapLayer1Texture;
    std::unique_ptr<GfxTexture> mapLayer2Texture;
    std::unique_ptr<GfxTexture> LonLatLookupTexture;

    std::vector<float> mesh;
    double sunTargetLat;
    double sunTargetLon;
    float sunPropAngleCurrent;
    float sunPropAngleTarget;
    double sunCurrentLat;
    double sunCurrentLon;
    bool overrideSunLocation;

    NaturalEarth projection;
    AstronomyService& astro;
};
