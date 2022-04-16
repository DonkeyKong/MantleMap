#include "Utils.hpp"
#include "ConfigService.hpp"
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

#include <fmt/format.h>
#include <sigslot/signal.hpp>

volatile bool interrupt_received = false;
volatile bool internal_exit = false;
static bool sleeping = false;

static ConfigService configService;
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

// static void executeCommand(const char* src, const char* cmd, ConfigService& configService)
// {
//     std::string command(cmd);
//     std::string source(src);

//     if (cmdDebugScenePtr != nullptr)
//     {
//       cmdDebugScenePtr->ShowCmd(command);
//     }

//     if (command == "exit")
//     {
//       std::cout << "Sending internal exit signal..." << std::endl;
//       internal_exit = true;
//     }
//     else if (command == "help")
//     {
//       printCmdMenu();
//     }
//     else if (command == "sleep")
//     {
//       sleeping = true;
//     }
//     else if (command == "light adjust on")
//     {
//       configService.lightAdjustEnabled = true;
//     }
//     else if (command == "light adjust off")
//     {
//       configService.lightAdjustEnabled = false;
//     }
//     else if (command.rfind("base layer ", 0) == 0)
//     {
//       std::string newSceneName = command.substr(11);
//       Scene* scene = getBaseSceneByName(newSceneName);
//       if (scene != nullptr)
//         swapBaseScene(scene->SceneName());
//       else
//         std::cout << "The requested base layer \"" << newSceneName << "\" does not exist." << std::endl;
//     }
//     else if (command == "reset")
//     {

//     }
//     else
//     {
//       // Unrecognized queries fall through in their entirety to the scenes
//       bool handled = false;
//       std::string result;

//       if (!handled)
//       {
//         for (Scene* scene : baseScenes)
//         {
//           if (scene->Query(command, result))
//           {
//             handled = true;
//             break;
//           }
//         }
//       }

//       if (!handled)
//       {
//         for (Scene* scene : overlayScenes)
//         {
//           if (scene->Query(command, result))
//           {
//             handled = true;
//             break;
//           }
//         }
//       }

//       std::cout << "[" << source << "]: ";

//       if (handled)
//         std::cout << result << std::endl;
//       else
//         std::cout << "Command \"" << command << "\" was not recognized." << std::endl;
//     }
// }

static void addScene(Scene* scene)
{
    // Subscribe the scene to system events
    sceneChanged.connect(&Scene::OnSceneChanged, scene);

    // Add the scene to the appropriate collection
    if (scene->GetSceneType() == SceneType::Base)
        baseScenes.push_back(scene);
    else
        overlayScenes.push_back(scene);
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

int main(int argc, char *argv[])
{
    configService.Init();

    // Subscribe to signal interrupts
    signal(SIGTERM, InterruptHandler);
    signal(SIGINT, InterruptHandler);

    // Add the HTTP service to serve web requests
    HttpService httpService(configService);

    // Init the astro / NOVAS lib
    AstronomyService astronomyService(configService);

    // Create the scene library
    DebugTransformScene debugScene(configService);
    LightScene lightScene(configService, astronomyService);
    MapTimeScene mapTimeScene(configService);
    WeatherScene weatherScene(configService);
    SolarScene solarScene(configService, astronomyService);
    ConfigCodeScene configScene(configService, httpService);
    PhysicsScene physicsScene(configService);

    addScene(&debugScene);
    addScene(&solarScene);
    addScene(&lightScene);
    addScene(&mapTimeScene);
    addScene(&weatherScene);
    addScene(&configScene);
    addScene(&physicsScene);

    // Make sure the default scene is first
    for (int i=0; i < baseScenes.size(); i++)
    {
        if (baseScenes[i]->SceneName() == configService.defaultScene)
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
    configService.SaveConfig();

    // Create our hardware accelerated renderer
    GLRenderContext render(configService);

    // Create the output display device (LED panel, window, etc)
    DisplayDevice display(configService);

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

    //   int buttonAction = 0;
    //   std::vector<std::string> buttonActions;

    //   for (const auto& scene : baseScenes)
    //   {
    //     buttonActions.push_back(fmt::format("base layer {}", scene->SceneName()));
    //   }
    //   buttonActions.push_back("sleep");
    //   buttonActions.push_back("reset");

    auto thisFrameComplete = std::chrono::high_resolution_clock::now();
    auto lastFrameComplete = std::chrono::high_resolution_clock::now();
    auto expectedFrameTime = std::chrono::high_resolution_clock::duration(std::chrono::nanoseconds(16666667));

    // Start the main render loop!
    while (!interrupt_received && !internal_exit)
    {
        // Handle input events
        //for (auto button : inputButtons)
        //{
            //   while(true)
            //   {
            //     auto action = button->PopAction();
            //     if (action == ButtonAction::None)
            //     {
            //       break;
            //     }
            //     else if (action == ButtonAction::Tap)
            //     {
            //       //executeCommand("Button", buttonActions[buttonAction].c_str(), configService);
            //       buttonAction = (buttonAction + 1) % buttonActions.size();
            //     }
            //     else if (action == ButtonAction::Hold)
            //     {
            //       //executeCommand("Button", "sleep", configService);
            //     }
            //     else if (action == ButtonAction::Exit)
            //     {
            //       internal_exit = true;
            //     }
            //   }
        //}

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

            // Regulate framerate to cap at 60 FPS
            thisFrameComplete = std::chrono::high_resolution_clock::now();
            std::this_thread::sleep_until(lastFrameComplete + expectedFrameTime);
            lastFrameComplete = thisFrameComplete;
        }
    }

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
