#pragma once

#include "Scene.hpp"
#include "Attributes.hpp"
#include "PolyFill.hpp"

class PhysicsScene : public Scene
{
 public:
    PhysicsScene();
    ~PhysicsScene();
    
    const char* SceneName() override;
    
private:
    virtual void initGLOverride() override;
    virtual void drawOverride() override;
    virtual void updateOverride() override;
    virtual void showOverride() override;

    PolyFill bgFill;
    std::vector<PhysicsPoint> points;
    int updateCounter;
};

