#include "Utils.hpp"
#include "ConfigService.hpp"
#include "TimeService.hpp"
#include "HttpService.hpp"
#include "AstronomyService.hpp"
#include "LoadShaders.hpp"
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

#define HTTP_SERVER_PORT 80

volatile bool interrupt_received = false;
volatile bool internal_exit = false;
static void InterruptHandler(int signo) 
{
  interrupt_received = true;
}

std::vector<Scene*> baseScenes;
std::vector<Scene*> overlayScenes;

static void addScene(Scene* scene)
{
  if (scene->GetSceneType() == SceneType::Base)
    baseScenes.push_back(scene);
  else
    overlayScenes.push_back(scene);
}

static Scene* getBaseSceneByName(std::string sceneName)
{
  for (Scene* scene : baseScenes) 
  {
    if (iequals(scene->SceneResourceDir(), sceneName))
      return scene;
  }
  
  return nullptr;
}

static Scene* getSceneByName(std::string sceneName)
{
  for (Scene* scene : baseScenes) 
  {
    if (iequals(scene->SceneResourceDir(), sceneName))
      return scene;
  }
  
  for (Scene* scene : overlayScenes) 
  {
    if (iequals(scene->SceneResourceDir(), sceneName))
      return scene;
  }
  
  return nullptr;
}

static void printCmdMenu()
{
  std::cout << std::endl;
  std::cout << "== Map Control Menu ==" << std::endl;
  std::cout << "Commands: base layer [sceneName], query [queryText], reset, sleep, exit" << std::endl;
  std::cout << "Base Layers: Light, Debug";
  std::cout << std::endl << std::endl;
}

static void swapBaseScene(std::string sceneName)
{
  Scene* newScene = getSceneByName(sceneName);
  if (newScene != nullptr)
  {
    for (Scene* scene : baseScenes) 
    {
      scene->Hide();
    }
    newScene->Show();
    
    for (Scene* scene : overlayScenes) 
    {
      scene->BaseSceneChanged(newScene->SceneResourceDir());
    }
  }
}

static CmdDebugScene* cmdDebugScenePtr = nullptr;
static bool sleeping = false;
static void executeCommand(const char* src, const char* cmd, ConfigService& configService)
{
    std::string command(cmd);    
    std::string source(src);
    
    if (cmdDebugScenePtr != nullptr)
    {
      cmdDebugScenePtr->ShowCmd(command);
    }
    
    if (command == "exit")
    {
      std::cout << "Sending internal exit signal..." << std::endl;
      internal_exit = true;
    }
    else if (command == "help")
    {
      printCmdMenu();
    }
    else if (command == "sleep")
    {
      sleeping = true;
    }
    else if (command == "light adjust on")
    {
      configService.lightAdjustEnabled = true;
    }
    else if (command == "light adjust off")
    {
      configService.lightAdjustEnabled = false;
    }
    else if (command.rfind("base layer ", 0) == 0)
    {
      std::string newSceneName = command.substr(11);
      Scene* scene = getBaseSceneByName(newSceneName);
      if (scene != nullptr)
        swapBaseScene(scene->SceneResourceDir());
      else
        std::cout << "The requested base layer \"" << newSceneName << "\" does not exist." << std::endl;
    }
    else if (command == "reset")
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
      swapBaseScene(configService.defaultScene);
    }
    else
    {
      // Unrecognized queries fall through in their entirety to the scenes
      bool handled = false;
      std::string result;
      
      if (!handled)
      {
        for (Scene* scene : baseScenes) 
        {
          if (scene->Query(command, result))
          {
            handled = true;
            break;
          }
        }
      }
      
      if (!handled)
      {
        for (Scene* scene : overlayScenes) 
        {
          if (scene->Query(command, result))
          {
            handled = true;
            break;
          }
        }
      }
      
      std::cout << "[" << source << "]: ";

      if (handled)
        std::cout << result << std::endl;
      else
        std::cout << "Command \"" << command << "\" was not recognized." << std::endl;
    }
}

int main(int argc, char *argv[]) 
{
  // Subscribe to signal interrupts
  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);
  
  // Create the settings object used throughout the components
  ConfigService configService;

  // Add the HTTP service to serve web requests
  HttpService httpService(configService);

  // Init the astro / NOVAS lib
  AstronomyService astronomyService(configService);
  
  // Create the scene library
  DebugTransformScene debugScene(configService);
  LightScene lightScene(configService, astronomyService);
  MapTimeScene mapTimeScene(configService);
  WeatherScene weatherScene(configService);
  CmdDebugScene cmdDebugScene(configService);
  SolarScene solarScene(configService, astronomyService);
  ConfigCodeScene configScene(configService, httpService);
  PhysicsScene physicsScene(configService);
  
  addScene(&debugScene);
  addScene(&solarScene);
  addScene(&lightScene);
  addScene(&mapTimeScene);
  addScene(&weatherScene);
  addScene(&cmdDebugScene);
  addScene(&configScene);
  addScene(&physicsScene);
  cmdDebugScenePtr = &cmdDebugScene;
  
  // Always show the following overlays
  mapTimeScene.Show();
  weatherScene.Show();
  cmdDebugScene.Show();
  
  // Bring up the default base scene
  swapBaseScene(configService.defaultScene);
  
  // Save the config after opening all the scenes
  configService.SaveConfig();

  // Create our hardware accelerated renderer
  GLRenderContext render(configService);

  // We might have many input buttons, create a vector to store them
  std::vector<InputButton*> inputButtons;

  // Create the output display device (LED panel, window, etc)
  DisplayDevice display(configService);
  if (display.GetInputButton() != nullptr)
  {
    inputButtons.push_back(display.GetInputButton());
  }

  // Create the button we listen to for sleep commands
  #ifdef LINUX_HID_CONTROLLER_SUPPORT
  UsbButton usbButton;
  inputButtons.push_back(&usbButton);
  #endif

  int buttonAction = 0;
  std::vector<std::string> buttonActions;

  for (const auto& scene : baseScenes)
  {
    buttonActions.push_back(fmt::format("base layer {}", scene->SceneResourceDir()));
  }
  buttonActions.push_back("sleep");
  buttonActions.push_back("reset");

  auto thisFrameComplete = std::chrono::high_resolution_clock::now();
  auto lastFrameComplete = std::chrono::high_resolution_clock::now();
  auto expectedFrameTime = std::chrono::high_resolution_clock::duration(std::chrono::nanoseconds(16666667));

  // Start the main render loop!
  while (!interrupt_received && !internal_exit) 
  {
    // Handle input events
    for (auto button : inputButtons)
    {
      while(true)
      {
        auto action = button->PopAction();
        if (action == ButtonAction::None)
        {
          break;
        }
        else if (action == ButtonAction::Tap)
        {
          executeCommand("Button", buttonActions[buttonAction].c_str(), configService);
          buttonAction = (buttonAction + 1) % buttonActions.size();
        }
        else if (action == ButtonAction::Hold)
        {
          executeCommand("Button", "sleep", configService);
        }
        else if (action == ButtonAction::Exit)
        {
          internal_exit = true;
        }
      }
    }
    
    // TBD: handle REST events
    while(true)
    {
      auto cmd = httpService.PopCommand();
      if (cmd.commandType == CommandType::TextCommand)
      {
        executeCommand("HTTP", cmd.commandStr.c_str(), configService);
      }
      else
      {
        break;
      }
    }
    
    // Update/Draw the map
    if (sleeping)
    {
      display.Clear();
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }  
    else
    {
      render.BeginDraw();
      
      for (Scene* scene : baseScenes) 
      {
        scene->Update();
      }

      for (Scene* scene : overlayScenes) 
      {
        scene->Update();
      }
      
      for (Scene* scene : baseScenes) 
      {
        scene->Draw();
        if (scene->Visible()) // Only draw one base layer
          break;
      }

      for (Scene* scene : overlayScenes) 
      {
        scene->Draw();
      }
      
      display.Update();

      // Regulate framerate to cap at 60 FPS
      thisFrameComplete = std::chrono::high_resolution_clock::now();
      std::this_thread::sleep_until(lastFrameComplete+expectedFrameTime);
      lastFrameComplete = thisFrameComplete;
    }

    // Evaluate message loop at 60FPS
    // std::this_thread::sleep_for(std::chrono::microseconds(16666));

    // std::string command;
    // //std::cout << "> " << std::flush; // This chatters in the logs
    // if (stdInFailCount < 20 && std::getline (std::cin, command))
    // {
    //   executeCommand("StdIn", command.c_str(), configService);
    // }
    // else
    // {
    //   if (stdInFailCount < 1000)
    //     stdInFailCount++;
    //   sleep(1); // sleep for a second after getting stdin fails
    // }
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
