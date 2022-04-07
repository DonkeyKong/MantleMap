#pragma once

#include <mutex>
#include <thread>
#include <condition_variable>

enum class CommandType
{
    None,
    TextCommand
}

struct HttpCommand
{
    CommandType commandType;
    std::string commandStr;
};

class HttpService
{
public:
    HttpService();
    bool ServerRunning();
    std::string ListeningInterface();
    HttpCommand PopCommand();
private:
    bool _exitThread;
    std::mutex _updateThreadMutex;
    std::condition_variable _exitThreadCondition;
    std::shared_ptr<std::thread> _updateThread;
};