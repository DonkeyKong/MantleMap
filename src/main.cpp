#include "ConfigService.hpp"
static auto& config = ConfigService::global;

#include "Utils.hpp"
#include "TimeService.hpp"
#include "HttpService.hpp"
#include "AstronomyService.hpp"
#include "GLRenderContext.hpp"
#include "Scene.hpp"
#include "LightScene.hpp"
#include "MapTimeScene.hpp"
#include "CmdDebugScene.hpp"
#include "ConfigCodeScene.hpp"
#include "DebugTransformScene.hpp"
#include "SolarScene.hpp"
#include "WeatherScene.hpp"
#include "TextLabel.hpp"
#include "DisplayDevice.hpp"
#include "InputButton.hpp"
#include "PhysicsScene.hpp"

#include <unistd.h>
#include <signal.h>
#include <thread>
#include <algorithm>
#include <map>
#include <string>
#include <vector>
#include <iostream>

#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <sigslot/signal.hpp>

using json = nlohmann::json;

static const std::string DEFAULT_SCENE_NAME = "Solar";
static const int DEFAULT_FPS = 60;

volatile bool interrupt_received = false;
volatile bool internal_exit = false;
static bool sleeping = false;
static auto expectedFrameTime = std::chrono::high_resolution_clock::duration::min();

static std::vector<Scene*> baseScenes;
static std::vector<Scene*> overlayScenes;

static sigslot::signal<std::string> sceneChanged;

static void InterruptHandler(int signo)
{
    interrupt_received = true;
}

static Scene* getSceneByName(std::string sceneName)
{
    for (Scene *scene : baseScenes)
    {
        if (iequals(scene->SceneName(), sceneName))
            return scene;
    }

    for (Scene *scene : overlayScenes)
    {
        if (iequals(scene->SceneName(), sceneName))
            return scene;
    }

    return nullptr;
}

static void showScene(int sceneIndex)
{
    Scene* newScene = baseScenes[sceneIndex];
    if (newScene != nullptr)
    {
        for (Scene* scene : baseScenes)
        {
            scene->Hide();
        }

        newScene->Show();

        sceneChanged(newScene->SceneName());
    }
}

static void showScene(std::string sceneName)
{
    Scene* newScene = getSceneByName(sceneName);
    if (newScene != nullptr)
    {
        for (Scene* scene : baseScenes)
        {
            scene->Hide();
        }

        newScene->Show();

        sceneChanged(newScene->SceneName());
    }
}

static bool showNext()
{
    bool showNext = false;
    for (Scene* scene : baseScenes)
    {
        if (showNext)
        {
            showScene(scene->SceneName());
            return true;
        }
        if (scene->Visible())
        {
            showNext = true;
        }
    }
    assert(showNext); // If nothing was showing, that's weird and we should know about it
    return false;
}

static void reset()
{
    sleeping = false;
    TimeService::ResetSceneTime();
    for (Scene* scene : baseScenes)
    {
        scene->Reset(true);
    }
    for (Scene* scene : overlayScenes)
    {
        scene->Reset(true);
    }
    showScene(0);
}

static void wake()
{
    sleeping = false;
}

static void sleep()
{
    sleeping = true;
}

static void addScene(Scene* scene, HttpService& http)
{
    // Subscribe the scene to system events
    sceneChanged.connect(&Scene::OnSceneChanged, scene);

    // Add the scene to the appropriate collection
    if (scene->GetSceneType() == SceneType::Base)
        baseScenes.push_back(scene);
    else
        overlayScenes.push_back(scene);

    // Register the scene with the server
    scene->RegisterEndpoints(http);
}

static void addButton(InputButton& b)
{
    b.OnTap.connect([&]()
    {
        // If we are asleep, wake
        if (sleeping)
        {
            reset();
        }
        // If we are awake, go to next scene
        else
        {
            // If this is the last scene, sleep
            if (!showNext())
            {
                sleeping = true;
            }
        }
    });

    b.OnHold.connect([&]()
    {
        // Go to sleep
        sleeping = true;
    });
}

void setupSystemHttpEndpoints(httplib::Server& srv)
{
    srv.Post("/system/sleep", [=](const httplib::Request& req, httplib::Response& res) 
    {
        sleep();
    });

    srv.Post("/system/reset", [=](const httplib::Request& req, httplib::Response& res) 
    {
        reset();
    });

    srv.Post("/system/restart", [=](const httplib::Request& req, httplib::Response& res) 
    {
        internal_exit = true;
    });

    srv.Patch("/system/settings", [=](const httplib::Request& req, httplib::Response& res) 
    {
        auto settingsPatch = json::parse(req.body);

        for (auto& kvp : settingsPatch.items())
        {
            if (!config.HasKey(kvp.key()))
            {
                res.status = 400;
                res.body = fmt::format("Bad patch request, settings key {} is invalid.", kvp.key());
                return;
            }

            if (!config.ValueTypeMatches(kvp.key(), kvp.value()))
            {
                res.status = 400;
                res.body = fmt::format("Bad patch request, value {} was an incorrect type.", kvp.key());
                return;
            }
        }

        for (auto& kvp : settingsPatch.items())
        {
            config.SetConfigValue(kvp.key(), kvp.value());
        }
    });

    srv.Get("/system/settings", [=](const httplib::Request& req, httplib::Response& res) 
    {
        std::stringstream ss;
        ss << std::setw(4) << config.GetConfigJson();
        res.body = ss.str();
    });

    srv.Get("/scenes", [=](const httplib::Request& req, httplib::Response& res) 
    {
        json scenes = json::array();
        for (const auto& scene : baseScenes)
        {
            scenes.push_back(scene->SceneName());
        }
        std::stringstream ss;
        ss << std::setw(4) << scenes;
        res.body = ss.str();
    });

    srv.Post(R"(/scenes/([a-zA-Z0-9]+)/show)", [=](const httplib::Request& req, httplib::Response& res) 
    {
        auto scene = getSceneByName(req.matches[1].str());
        if (scene == nullptr)
        {
            res.status = 404;
            res.body = fmt::format("The scene {} was not found.", req.matches[1].str());
            return;
        }
        showScene(scene->SceneName());
    });
}

int main(int argc, char *argv[])
{
    // Subscribe to signal interrupts
    signal(SIGTERM, InterruptHandler);
    signal(SIGINT, InterruptHandler);

    // Init the config service here (since doing it in static init is disallowed)
    // and because lots of components rely on its basic vars being set
    config.Init();

    std::string defaultScene = DEFAULT_SCENE_NAME;
    int fpsLimit = DEFAULT_FPS;
    
    // Subscribe to settings changes (this also runs the lambda once before subscribing)
    config.Subscribe([&](std::string setting)
    {
        config.UpdateIfChanged(defaultScene, setting, "defaultScene", DEFAULT_SCENE_NAME);
        
        if (config.UpdateIfChanged(fpsLimit, setting, "fpsLimit", DEFAULT_FPS))
        {
            expectedFrameTime = std::chrono::high_resolution_clock::duration(
            std::chrono::nanoseconds((int)(1.0/(double)fpsLimit * 1000000000.0))    );
        }
    });

    // Add the HTTP service to serve web requests
    HttpService httpService;
    setupSystemHttpEndpoints(httpService.Server());

    // Init the astro / NOVAS lib
    AstronomyService astronomyService;

    // Create the scene library
    DebugTransformScene debugScene;
    LightScene lightScene(astronomyService);
    MapTimeScene mapTimeScene;
    WeatherScene weatherScene;
    SolarScene solarScene(astronomyService);
    ConfigCodeScene configScene(httpService);
    PhysicsScene physicsScene;

    addScene(&debugScene, httpService);
    addScene(&solarScene, httpService);
    addScene(&lightScene, httpService);
    addScene(&mapTimeScene, httpService);
    addScene(&weatherScene, httpService);
    addScene(&configScene, httpService);
    addScene(&physicsScene, httpService);

    
    for (int i=0; i < baseScenes.size(); i++)
    {
        if (baseScenes[i]->SceneName() == defaultScene)
        {
            if (i != 0)
            {
                std::swap(baseScenes[0], baseScenes[i]);
            }
            break;
        }
    }

    // Always show the following overlays
    mapTimeScene.Show();
    weatherScene.Show();

    // Bring up the first base scene
    showScene(0);

    // Save the config after opening all the scenes
    config.SaveConfig();

    // Create our hardware accelerated renderer
    GLRenderContext render;

    // Create the output display device (LED panel, window, etc)
    DisplayDevice display;

    // Connect display events
    display.OnDisconnect.connect([](){internal_exit = true;});
    if (display.GetInputButton() != nullptr)
    {
        addButton(*display.GetInputButton());
    }

// Create the button we listen to for sleep commands
#ifdef LINUX_HID_CONTROLLER_SUPPORT
    UsbButton usbButton;
    addButton(usbButton);
#endif
    auto thisFrameComplete = std::chrono::high_resolution_clock::now();
    auto lastFrameComplete = std::chrono::high_resolution_clock::now();
    // Start the main render loop!
    while (!interrupt_received && !internal_exit)
    {
        // Update/Draw the map
        if (sleeping)
        {
            display.Clear();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        else
        {
            render.BeginDraw();

            for (Scene *scene : baseScenes)
            {
                scene->Update();
            }

            for (Scene *scene : overlayScenes)
            {
                scene->Update();
            }

            for (Scene *scene : baseScenes)
            {
                scene->Draw();
                if (scene->Visible()) // Only draw one base layer
                    break;
            }

            for (Scene *scene : overlayScenes)
            {
                scene->Draw();
            }

            display.Update();

            // Regulate framerate
            thisFrameComplete = std::chrono::high_resolution_clock::now();
            std::this_thread::sleep_until(lastFrameComplete + expectedFrameTime);
            lastFrameComplete = thisFrameComplete;
        }
    }

    config.SaveConfig();

    if (interrupt_received)
    {
        fprintf(stderr, "Main thread caught exit signal.\n");
    }

    if (internal_exit)
    {
        fprintf(stderr, "Main thread got internal exit request.\n");
    }

    return 0;
}
