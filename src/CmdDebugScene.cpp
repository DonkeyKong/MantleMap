#include "CmdDebugScene.hpp"

CmdDebugScene::CmdDebugScene(ConfigService& map) : Scene(map, SceneType::Overlay, SceneLifetime::Manual),
    _cmdLabel(map)
{
  _cmdShowCounter = 0;
  _framesToHoldCmd = 60;
  _framesToScrollCmd = 60;
}

CmdDebugScene::~CmdDebugScene()
{
}

void CmdDebugScene::initGLOverride()
{
}

const char* CmdDebugScene::SceneName()
{
  return "CmdDebug";
}

void CmdDebugScene::updateOverride()
{
  if (_cmdShowCounter < (_framesToHoldCmd + _framesToScrollCmd))
  {
    _cmdShowCounter++;
    
    int showPos = 0;
  
    if (_cmdShowCounter > _framesToHoldCmd)
    {
      float distPerFrame = (float)_cmdLabel.GetLength() / (float)_framesToScrollCmd;
      showPos = (int)(distPerFrame * (float)(_cmdShowCounter - _framesToHoldCmd));
    }
    
    _cmdLabel.SetPosition(-showPos, 0);
  }
}

void CmdDebugScene::ShowCmd(std::string query)
{
  _cmdLabel.SetText(query);
  _cmdLabel.SetFontStyle(FontStyle::Narrow);
  _cmdLabel.SetColor(0.25, 0.25, 0.25, 1.0);
  
  _cmdShowCounter = 0;
  _framesToScrollCmd = _cmdLabel.GetLength() * 3;
}

void CmdDebugScene::drawOverride()
{
  if (_cmdShowCounter < (_framesToHoldCmd + _framesToScrollCmd))
	  _cmdLabel.Draw();
}
