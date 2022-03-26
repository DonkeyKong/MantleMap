#include "Utils.h"
#include "MapState.hpp"
#include "LoadShaders.hpp"
#include "GLRenderContext.hpp"
#include "Scene.hpp"
#include "LightScene.hpp"
#include "MapTimeScene.hpp"
#include "CmdDebugScene.hpp"
#include "DebugTransformScene.hpp"
#include "SolarScene.hpp"
#include "WeatherScene.hpp"
#include "TextLabel.hpp"
#include "DisplayDevice.hpp"
#include "UsbButton.hpp"

#include <unistd.h>
#include <signal.h>
#include <thread>
#include <algorithm>
#include <map>
#include <string>
#include <vector>
#include <iostream>

#define HTTP_SERVER_PORT 80

volatile bool interrupt_received = false;
volatile bool internal_exit = false;
static void InterruptHandler(int signo) 
{
  interrupt_received = true;
}

std::vector<Scene*> baseScenes;
std::vector<Scene*> overlayScenes;
CmdDebugScene* cmdDebugScenePtr;

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

static void executeCommand(const char* src, const char* cmd, MapState& mapState)
{
    std::string command(cmd);    
    std::string source(src);
    
    cmdDebugScenePtr->ShowCmd(command);
    
    std::cout << "[" << source << "]: ";
    
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
      mapState.SetSleep(true);
    }
    else if (command == "light adjust on")
    {
      mapState.lightAdjustEnabled = true;
    }
    else if (command == "light adjust off")
    {
      mapState.lightAdjustEnabled = false;
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
      mapState.SetSleep(false);
      mapState.ResetTime();
      for (Scene* scene : baseScenes) 
      {
          scene->Reset(true);
      }
      for (Scene* scene : overlayScenes) 
      {
        scene->Reset(true);
      }
      swapBaseScene(mapState.defaultScene);
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
      
      if (handled)
        std::cout << result << std::endl;
      else
        std::cout << "Command \"" << command << "\" was not recognized." << std::endl;
    }
}

void renderThread(MapState* map, const RGBMatrix::Options& matrixParams, const rgb_matrix::RuntimeOptions& runtimeParams)
{  
  // Create the output display device (LED panel, window, etc)
  DisplayDevice display(*map);
  
  // Create our hardware accelerated renderer
  GLRenderContext render(*map);

  // Create the button we listen to for sleep commands
  UsbButton button;
  
  for (Scene* scene : baseScenes) 
  {
    scene->InitGL();
  }

  for (Scene* scene : overlayScenes) 
  {
    scene->InitGL();
  }

  while (!interrupt_received && !internal_exit) 
  {
    while (button.pressed())
    {
      if (map->GetSleep())
      {
        // Wake and reset if sleeping
        executeCommand("Button", "reset", *map);
      }
      else
      {
        // Sleep if awake
        executeCommand("Button", "sleep", *map);
      }
    }
    
    if (map->GetSleep())
    {
      display.Sleep();
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
    }
  }

  if (interrupt_received) 
  {
    fprintf(stderr, "Render thread caught signal. Exiting.\n");
  }
  
  if (internal_exit) 
  {
    fprintf(stderr, "Render thread got internal exit request\n");
  }
}

int main(int argc, char *argv[]) 
{
  // Subscribe to signal interrupts
  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);
  
  MapState mapState;
  
  // Mantle map parameters
  int cols = 64;
  int rows = 32;
  int chainLength = 3;
  int parallelLength = 3;
  int slowdown = 2;
  
  // Move params into structures
  RGBMatrix::Options matrixParams;
  //std::string hardware_mapping = "regular";
  matrixParams.rows = rows;
  matrixParams.cols = cols;
  matrixParams.chain_length = chainLength;
  matrixParams.hardware_mapping = "regular";
  matrixParams.parallel = parallelLength;
  matrixParams.pwm_lsb_nanoseconds = 200;
  matrixParams.brightness = 100;
  matrixParams.pwm_bits = 6;
  rgb_matrix::RuntimeOptions runtimeParams;
  runtimeParams.do_gpio_init = true;
  runtimeParams.gpio_slowdown = slowdown;
  
  // Create the scene library
  DebugTransformScene debugScene(mapState);
  LightScene lightScene(mapState);
  MapTimeScene mapTimeScene(mapState);
  WeatherScene weatherScene(mapState);
  CmdDebugScene cmdDebugScene(mapState);
  SolarScene solarScene(mapState);
  
  addScene(&debugScene);
  addScene(&solarScene);
  addScene(&lightScene);
  addScene(&mapTimeScene);
  addScene(&weatherScene);
  addScene(&cmdDebugScene);
  cmdDebugScenePtr = &cmdDebugScene;
  
  // By default, always show mapTime, weather, and cmdDebug
  mapTimeScene.Show();
  weatherScene.Show();
  cmdDebugScene.Show();
  
  // Bring up the default base scene
  swapBaseScene(mapState.defaultScene);
  
  // Save the config after opening all the scenes
  mapState.SaveConfig();
  
  std::thread t(&renderThread, &mapState, matrixParams, runtimeParams);
  
  printCmdMenu();
  int stdInFailCount = 0;
  
  while (!interrupt_received && !internal_exit) 
  {
    std::string command;
    //std::cout << "> " << std::flush; // This chatters in the logs
    if (stdInFailCount < 20 && std::getline (std::cin, command))
    {
      executeCommand("StdIn", command.c_str(), mapState);
    }
    else
    {
      if (stdInFailCount < 1000)
        stdInFailCount++;
      sleep(1); // sleep for a second after getting stdin fails
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
  
  t.join();

  return 0;
}
