#pragma once

#include "Scene.hpp"
#include "Attributes.hpp"

class PhysicsScene : public Scene
{
 public:
    PhysicsScene(MapState& map);
    ~PhysicsScene();
    
    const char* SceneName() override;
    const char* SceneResourceDir() override;
    
private:
    virtual void initGLOverride() override;
    virtual void drawOverride() override;
    virtual void updateOverride() override;
    virtual void showOverride() override;


    std::vector<PhysicsPoint> points;
};

