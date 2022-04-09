#pragma once

#include <ConfigService.hpp>

#include <httplib.h>
#include <vector>
#include <mutex>
#include <thread>
#include <condition_variable>

enum class CommandType
{
    None,
    TextCommand
};

struct HttpCommand
{
    CommandType commandType {CommandType::None};
    std::string commandStr {""};
};

class HttpService
{
public:
    HttpService(ConfigService& config);
    ~HttpService();
    bool ServerRunning();
    std::string ListeningInterface();
    HttpCommand PopCommand();
private:
    std::string listeningInterface;
    ConfigService& config;
    void setupCallbacks();
    std::unique_ptr<httplib::Server> srv;
    std::unique_ptr<std::thread> serverThread;
    std::mutex queueMutex;
    std::vector<HttpCommand> commandQueue;
    std::unordered_map<std::string, std::string> web;
};