#ifndef WeatherScene_HPP
#define WeatherScene_HPP

#include "TextLabel.hpp"
#include "Scene.hpp"
#include "MapState.hpp"

#include <mutex>
#include <thread>
#include <condition_variable>

class WeatherScene : public Scene
{
 public:
    WeatherScene(MapState& map);
    ~WeatherScene();
    
    const char* SceneName() override;
    const char* SceneResourceDir() override;

protected:
    void initGLOverride() override;
    void updateOverride() override;
    void drawOverride() override;
    
private:
    
    std::string _noaaTemp;
    bool _exitTempUpdateThread;
    std::mutex _updateThreadMutex;
    std::condition_variable _exitThreadCondition;
    std::shared_ptr<std::thread> _tempUpdateThread;
    
    TextLabel _tempLabel;
};

#endif
