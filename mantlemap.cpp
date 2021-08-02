// shader-test is ©2018 Zack Schilling and nobody else

#include "led-matrix.h"
#include "pixel-mapper.h"
#include "content-streamer.h"
#include "LoadShaders.hpp"
#include "GLRenderContext.hpp"
#include "Scene.hpp"
#include "LightScene.hpp"
#include "MapTimeScene.hpp"
#include "CmdDebugScene.hpp"
#include "DebugTransformScene.hpp"
#include "SolarScene.hpp"
#include "MapState.hpp"
#include "TextLabel.hpp"

#include <unistd.h>
#include <signal.h>
#include <thread>
#include <algorithm>
#include <map>
#include <string>
#include <vector>
#include <iostream>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <Magick++.h>
#include <magick/image.h>

#define REMOTE_SERVER_PORT 34187

using rgb_matrix::GPIO;
using rgb_matrix::Canvas;
using rgb_matrix::FrameCanvas;
using rgb_matrix::RGBMatrix;
using rgb_matrix::StreamReader;

volatile bool interrupt_received = false;
volatile bool internal_exit = false;

std::vector<Scene*> baseScenes;
std::vector<Scene*> overlayScenes;
CmdDebugScene* cmdDebugScenePtr;

static void InterruptHandler(int signo) 
{
  interrupt_received = true;
}

static bool iequals(const std::string& a, const std::string& b)
{
    return std::equal(a.begin(), a.end(),
                      b.begin(), b.end(),
                      [](char a, char b) {
                          return tolower(a) == tolower(b);
                      });
}

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
  // Prepare matrix
  RGBMatrix* matrix = CreateMatrixFromOptions(matrixParams, runtimeParams);
  if (matrix == NULL)
  {
    internal_exit = true;
    return;
  }
  
  // Create our double buffering canvas
  FrameCanvas* offscreen_canvas = matrix->CreateFrameCanvas();
  
  // Create our hardware accelerated renderer
  GLRenderContext render(*map);
  
  for (Scene* scene : baseScenes) 
  {
    scene->InitGL();
  }

  for (Scene* scene : overlayScenes) 
  {
    scene->InitGL();
  }
  
    // Server vars
    int sd, rc, n;
    socklen_t cliLen;
    struct sockaddr_in cliAddr, remoteServAddr;
    struct hostent *h;
    int broadcast;
    char msg[1024];
    char src[256] = "UDP Device";
    struct timeval tv;
    
    /* get server IP address (no check if input is IP address or DNS name */
    broadcast = 1;

    h = gethostbyname("255.255.255.255");
    if(h==NULL) 
      printf("Unknown host '%s' \n", "255.255.255.255");

    //printf("Sending data to '%s' (IP : %s) \n", h->h_name, inet_ntoa(*(struct in_addr *)h->h_addr_list[0]));

    remoteServAddr.sin_family = h->h_addrtype;
    memcpy((char *) &remoteServAddr.sin_addr.s_addr, 
       h->h_addr_list[0], h->h_length);
    remoteServAddr.sin_port = htons(REMOTE_SERVER_PORT);

    /* socket creation */
    sd = socket(AF_INET,SOCK_DGRAM,0);
    if(sd<0)
      printf("Cannot open socket \n");

    if (setsockopt(sd, SOL_SOCKET, SO_BROADCAST, &broadcast,sizeof broadcast) == -1) 
      perror("setsockopt (SO_BROADCAST)");
      
    fcntl(sd, F_SETFL, O_NONBLOCK); 
    
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    if (setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
        perror("Error");

    /* bind any port */
    cliAddr.sin_family = AF_INET;
    cliAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    cliAddr.sin_port = htons(REMOTE_SERVER_PORT);
    cliLen = sizeof(cliAddr);

    rc = bind(sd, (struct sockaddr *) &cliAddr, sizeof(cliAddr));
    if (rc < 0) 
      printf("Cannot bind port\n");


  while (!interrupt_received && !internal_exit) 
  {
    n = recvfrom(sd, msg, 1023, 0, (struct sockaddr *) &cliAddr, &cliLen);
    if(n > 0) 
    {
      msg[n] = '\0';
      executeCommand(src, msg, *map);
    }
    
    if (map->GetSleep())
    {
      matrix->Clear();
      std::this_thread::sleep_for (std::chrono::milliseconds(100));
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
      
      render.FinishDraw(offscreen_canvas);
      offscreen_canvas = matrix->SwapOnVSync(offscreen_canvas);
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
  
  // Close the UDP listen port
  close(sd);

  // Animation finished. Shut down the RGB matrix.
  matrix->Clear();
  delete matrix;
}

int main(int argc, char *argv[]) 
{ 
  // Init ImageMagick library
  Magick::InitializeMagick(nullptr); //*argv);

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
  CmdDebugScene cmdDebugScene(mapState);
  SolarScene solarScene(mapState);
  
  addScene(&debugScene);
  addScene(&solarScene);
  addScene(&lightScene);
  addScene(&mapTimeScene);
  addScene(&cmdDebugScene);
  cmdDebugScenePtr = &cmdDebugScene;
  
  // By default, always show mapTime and cmdDebug
  mapTimeScene.Show();
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
