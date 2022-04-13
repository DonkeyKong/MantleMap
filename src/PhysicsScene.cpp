#include "PhysicsScene.hpp"
#include "GfxProgram.hpp"
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
        points[i].size = Random(0.1f, 1.5f);
        points[i].mass = pow(points[i].size, 2.0f) * 8.0f; //Random(10.0f, 20.0f);
        points[i].velocity = Random(Position{-0.1f, -0.1f, 0.0f}, Position{0.1f, 0.1f, 0.0f});
        points[i].color = Random(HSVColor(0.0f, 0.0f, 0.1f, 1.0f), HSVColor(360.0f, 0.6f, 1.0f, 1.0f));
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


static std::unique_ptr<GfxProgram> program;
//static GLint vertexAttrib, colorAttrib, pointSizeAttrib;
void PhysicsScene::initGLOverride()
{
    if (!program)
    {
        // Load and compile the shaders into a glsl program
        program = loadProgram("particlevert.glsl", "fragshader.glsl", { ShaderFeature::VertexColor });
    }
}

void PhysicsScene::drawOverride()
{
    if (points.size() > 0)
    {
        // Draw the particles
        program->Use();

        glVertexAttribPointer(
                    program->Attrib("aPosition"),      // The attribute ID
                    3,                  // size
                    GL_FLOAT,           // type
                    GL_FALSE,           // normalized?
                    sizeof(PhysicsPoint),                  // stride
                    points.data()         // underlying data
        );
        glEnableVertexAttribArray ( program->Attrib("aPosition") );
        
        glVertexAttribPointer(
                            program->Attrib("aColor"), // The attribute ID
                            4,                  // size
                            GL_FLOAT,           // type
                            GL_FALSE,           // normalized?
                            sizeof(PhysicsPoint),   // stride
                            ((float*)points.data())+3       // underlying data
        );
        glEnableVertexAttribArray(program->Attrib("aColor"));

        glVertexAttribPointer(
                            program->Attrib("aPointSize"), // The attribute ID
                            1,                  // size
                            GL_FLOAT,           // type
                            GL_FALSE,           // normalized?
                            sizeof(PhysicsPoint),   // stride
                            ((float*)points.data())+7      // underlying data
        );
        glEnableVertexAttribArray(program->Attrib("aPointSize"));

        // Draw the points!
        glDrawArrays(GL_POINTS, 0, points.size());
    }
}

static inline bool gravForce(const PhysicsPoint& p1, const PhysicsPoint& p2, Vec3& a1, Vec3& a2)
{
    const float G = 0.01f;
    const float sDist = 0.05;

    float d2 = (p1.pos-p2.pos).mag2() * sDist * sDist;
    float d = sqrt(d2);

    if (d < (p1.size+p2.size / 3.0f))
    {
        a1.x = 0.0f;
        a1.y = 0.0f;
        a2.x = 0.0f;
        a2.y = 0.0f;
        return true;
    }

    // F = m1*m2 / dist^2
    float F = p1.mass*p2.mass / d2 * G;
    Vec3 norm1 = (p2.pos-p1.pos)/d;
    Vec3 norm2 = (p1.pos-p2.pos)/d;
    a1 = norm1 * F * (p2.mass) / (p1.mass+p2.mass);
    a2 = norm2 * F * (p1.mass) / (p1.mass+p2.mass);
    return false;
}

static inline void merge(PhysicsPoint& p1, PhysicsPoint& p2)
{
    float totalMass = p1.mass+p2.mass;

    p1.pos = p1.pos * (p1.mass/totalMass) + p2.pos * (p2.mass/totalMass);
    p1.size = sqrt(pow(p1.size,2.0f) + pow(p2.size,2.0f));
    p1.velocity = p1.velocity * (p1.mass/totalMass) + p2.velocity * (p2.mass/totalMass);

    p1.mass = p1.mass + p2.mass;
    p2.size = 0;
    p2.mass = 0;
    p2.velocity = {0,0,0};
}

void PhysicsScene::updateOverride()
{
    const float dT = 0.004;

    updateCounter = (updateCounter + 1) % 30000;

    PhysicsPoint phantom
    {
        {(float)config.width / 2.0f, (float)config.height / 2.0f, 0.0f},
        {},
        0,
        100.0,
        {}
    };

    Vec3 a1, a2;
    for (int i=0; i < points.size(); i++)
    {
        if (points[i].mass == 0)
            continue;

        for (int j=i+1; j < points.size(); j++)
        {
            if (points[j].mass == 0)
                continue;

            bool mergePoints = gravForce(points[i], points[j], a1, a2);

            // if (mergePoints)
            // {
            //     merge(points[i], points[j]);
            // }
            // else
            {
                points[i].velocity += a1 * dT;
                points[j].velocity += a2 * dT;
            }
        }

        // Apply the phantom centering acceleration
        gravForce(points[i], phantom, a1, a2);
        points[i].velocity += a1 * dT;

        // Move the point for this time step
        points[i].pos += points[i].velocity * dT;
    }
}
