#ifndef WeatherScene_HPP
#define WeatherScene_HPP

#include "TextLabel.hpp"
#include "Scene.hpp"

#include <mutex>
#include <thread>
#include <condition_variable>

class WeatherScene : public Scene
{
 public:
    WeatherScene();
    ~WeatherScene();
    
    const char* SceneName() override;

protected:
    void updateOverride() override;
    
private:
    std::string _noaaTemp;
    bool _exitTempUpdateThread;
    std::mutex _updateThreadMutex;
    std::condition_variable _exitThreadCondition;
    std::shared_ptr<std::thread> _tempUpdateThread;
    
    TextLabel _tempLabel;
};

#endif
