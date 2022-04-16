#pragma once

#include "Scene.hpp"
#include "TextLabel.hpp"

class DebugTransformScene : public Scene
{
 public:
    DebugTransformScene(ConfigService& map);
    ~DebugTransformScene();
    
    const char* SceneName() override;

protected:
    void initGLOverride() override;
    void drawOverride() override;
    
private:
    std::unique_ptr<GfxProgram> program;
    
    TextLabel _label1;
    TextLabel _label2;
    TextLabel _label3;
    TextLabel _label4;
    TextLabel _label5;
    TextLabel _label6;
    TextLabel _label7;
    TextLabel _label8;

    // Helper function that draws a fullscreen rect, generally used to draw the map
    void drawMapRect();
    NaturalEarth projection;
    std::unique_ptr<GfxTexture> LonLatLookupTexture;
    std::vector<float> mesh;
};
