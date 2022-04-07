#include "PhysicsScene.hpp"
#include "LoadShaders.hpp"
#include "Utils.hpp"

#include <chrono>
#include <ctime>

PhysicsScene::PhysicsScene(ConfigService& map) : Scene(map, SceneType::Base, SceneLifetime::Manual),
    points(150)
{
    srand(time(nullptr));
}

void PhysicsScene::showOverride()
{
    updateCounter = 0;
    // Initialize the list of particles
    for (int i=0; i < points.size(); i++)
    {
        points[i].pos = Random(Position{0.0f, 0.0f, 0.0f}, Position{(float)config.width, (float)config.height, 0.0f});
        points[i].size = Random(0.5f, 3.0f);
        points[i].mass = points[i].size * 5.0f; //Random(10.0f, 20.0f);
        points[i].velocity = Random(Position{-0.05f, -0.05f, 0.0f}, Position{0.05f, 0.05f, 0.0f});
        points[i].color = Random(HSVColor(0.0f, 1.0f, 0.5f, 1.0f), HSVColor(360.0f, 1.0f, 0.6f, 1.0f));
    }
}

PhysicsScene::~PhysicsScene()
{
}

const char* PhysicsScene::SceneName()
{
  return "Physics";
}

const char* PhysicsScene::SceneResourceDir()
{
  return "Physics";
}


static GfxProgram program;
static GLint vertexAttrib, colorAttrib, pointSizeAttrib;
void PhysicsScene::initGLOverride()
{
    if (!program.isLoaded)
    {
        // Load and compile the shaders into a glsl program
        program = loadGraphicsProgram("particlevert.glsl", "particlefrag.glsl");
        program.SetCameraFromPixelTransform(config.width,config.height);
        vertexAttrib = glGetAttribLocation(program.GetId(), "aVertex");
        colorAttrib = glGetAttribLocation(program.GetId(), "aColor");
        pointSizeAttrib = glGetAttribLocation(program.GetId(), "aPointSize");
    }
}

void PhysicsScene::drawOverride()
{
    if (points.size() > 0)
    {
        // Draw the particles
        glUseProgram(program.GetId());

        glVertexAttribPointer(
                    vertexAttrib,      // The attribute ID
                    3,                  // size
                    GL_FLOAT,           // type
                    GL_FALSE,           // normalized?
                    sizeof(PhysicsPoint),                  // stride
                    points.data()         // underlying data
        );
        glEnableVertexAttribArray ( vertexAttrib );
        
        glVertexAttribPointer(
                            colorAttrib, // The attribute ID
                            4,                  // size
                            GL_FLOAT,           // type
                            GL_FALSE,           // normalized?
                            sizeof(PhysicsPoint),   // stride
                            ((float*)points.data())+3       // underlying data
        );
        glEnableVertexAttribArray(colorAttrib);

        glVertexAttribPointer(
                            pointSizeAttrib, // The attribute ID
                            1,                  // size
                            GL_FLOAT,           // type
                            GL_FALSE,           // normalized?
                            sizeof(PhysicsPoint),   // stride
                            ((float*)points.data())+7      // underlying data
        );
        glEnableVertexAttribArray(pointSizeAttrib);

        // Draw the points!
        glDrawArrays(GL_POINTS, 0, points.size());
    }
}

static inline void gravForce(const Position& p1, const float& m1, const Position& p2, const float& m2, Position& a1, Position& a2)
{
    const float G = 0.001f;
    // F = m1*m2 / dist^2
    float d2 = pow(p1.x-p2.x,2.0f) +  pow(p1.y-p2.y,2.0f);
    if (d2 < 1.0f)
    {
        a1.x = 0.0f;
        a1.y = 0.0f;
        a2.x = 0.0f;
        a2.y = 0.0f;
    }
    else
    {
        float d = sqrt(d2);
        float a1Mag = m2/d2*G;
        float a2Mag = m1/d2*G;
        a1.x = (p2.x - p1.x) / d * a1Mag;
        a1.y = (p2.y - p1.y) / d * a1Mag;
        a2.x = (p1.x - p2.x) / d * a2Mag;
        a2.y = (p1.y - p2.y) / d * a2Mag;
    }
}

void PhysicsScene::updateOverride()
{
    updateCounter = (updateCounter + 1) % 30000;

    PhysicsPoint phantom
    {
        {(float)config.width / 2.0f, (float)config.height / 2.0f, 0.0f},
        {},
        0,
        5.0,
        {}
    };

    Position a1, a2;
    for (int i=0; i < points.size(); i++)
    {
        for (int j=i+1; j < points.size(); j++)
        {
            gravForce(points[i].pos, points[i].mass, points[j].pos, points[j].mass, a1, a2);
            points[i].velocity.x += a1.x;
            points[i].velocity.y += a1.y;
            points[j].velocity.x += a2.x;
            points[j].velocity.y += a2.y;
        }

        // Apply the phantom centering acceleration
        gravForce(points[i].pos, points[i].mass, phantom.pos, phantom.mass, a1, a2);
        points[i].velocity.x += a1.x;
        points[i].velocity.y += a1.y;

        // Move the point for this time step
        points[i].pos.x += points[i].velocity.x;
        points[i].pos.y += points[i].velocity.y;
    }
}
