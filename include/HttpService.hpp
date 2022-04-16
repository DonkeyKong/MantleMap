#pragma once

#include <ConfigService.hpp>

#include <httplib.h>
#include <vector>
#include <mutex>
#include <thread>
#include <condition_variable>

class HttpService
{
public:
    HttpService(ConfigService& config);
    ~HttpService();
    bool Running();
    std::string ListeningInterface();
    httplib::Server& Server();
private:
    std::string listeningInterface;
    ConfigService& config;
    void setupCallbacks();
    std::unique_ptr<httplib::Server> srv;
    std::unique_ptr<std::thread> serverThread;
    std::unordered_map<std::string, std::string> web;
};