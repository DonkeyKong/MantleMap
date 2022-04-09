#include "HttpService.hpp"

#include <sys/types.h>
#include <ifaddrs.h>
#include <filesystem>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

static std::string getFirstExternalHostAddr()
{
    std::string hostAddr = "0.0.0.0";

    struct ifaddrs *ifaddr;
    char host[NI_MAXHOST];
    const char* loopbackPrefix = "lo";

    if (getifaddrs(&ifaddr) != -1) 
    {
        /* Walk through linked list, maintaining head pointer so we
            can free list later. */

        for (struct ifaddrs *ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) 
        {
            // Skip any interfaces without addresses
            if (ifa->ifa_addr == nullptr)
                continue;

            // Skip interfaces that aren't IPv4
            if (ifa->ifa_addr->sa_family != AF_INET)
                continue;

            // Skip loopback interfaces (we need to connect form outside!)
            bool isLoopback = strncmp(loopbackPrefix, ifa->ifa_name, strlen(loopbackPrefix)) == 0;
            if (isLoopback)
                continue;
                
            int s = getnameinfo(ifa->ifa_addr,
                    sizeof(struct sockaddr_in),
                    host, NI_MAXHOST,
                    NULL, 0, NI_NUMERICHOST);
            if (s == 0) 
            {
                // Return the first non-loopback, assigned interface
                hostAddr = std::string(host);
                break;
            }
        }
        freeifaddrs(ifaddr);
    }

    return hostAddr;
}

HttpService::HttpService(ConfigService& config) : config(config)
{
    std::filesystem::path webDir = std::filesystem::path(config.sceneResourcePath) / "Web";
    for (const auto & entry : std::filesystem::directory_iterator(webDir))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".html")
        {
            std::ifstream t(entry.path());
            std::stringstream buffer;
            buffer << t.rdbuf();
            web[entry.path().filename()] = buffer.str();
        }
    }
        
    srv = std::make_unique<httplib::Server>();

    // Setup the HTTP API
    setupCallbacks();

    // Figure out the port and bind address for the server
    int port = config.GetConfigValue("httpServicePort", 80);
    std::string addr = config.GetConfigValue("httpServiceAddress", std::string("0.0.0.0"));
    
    if (port == 0)
    {
      port = srv->bind_to_any_port(addr.c_str());

      serverThread = std::make_unique<std::thread>([=]() 
      {
        srv->listen_after_bind();
      });
      
    }
    else
    {
      serverThread = std::make_unique<std::thread>([=]() 
      {
        srv->listen(addr.c_str(), port);
      });
    }

    // Wait for server to start
    int i=0;
    while (srv->is_valid() && !srv->is_running() && i < 10)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if (!srv->is_valid() || i == 10)
    {
      srv = nullptr;
      serverThread = nullptr;
      throw std::runtime_error("Mock server did not start!");
    }

    listeningInterface = getFirstExternalHostAddr();
}

HttpService::~HttpService()
{
    if (srv != nullptr)
    {
        srv->stop();
    }

    if (serverThread != nullptr)
    {
        serverThread->join();
    }
}

static json getRequestPayload(const httplib::Request& req)
{
    json payload;
    if (req.has_header("Content-Type") &&
        req.get_header_value("Content-Type") == "application/x-www-form-urlencoded")
    {
        for (const auto& [key, value] : req.params)
        {
            payload[key] = value;
        }
    }
    else // assume json
    {
        payload = json::parse(req.body);
    }
    return payload;
}

void HttpService::setupCallbacks()
{
    srv->Get("/", [=](const httplib::Request& req, httplib::Response& res) 
    {
        res.set_content(web["index.html"], "text/html");
    });

    srv->Post("/system/command", [=](const httplib::Request& req, httplib::Response& res) 
    {
        json payload = getRequestPayload(req);

        if (payload.find("commandType") != payload.end() &&
            payload.find("commandStr") != payload.end())
        {
            HttpCommand cmd
            {
                payload["commandType"] == "TextCommand" ? CommandType::TextCommand : CommandType::None,
                payload["commandStr"]
            };

            {
                auto lock = std::scoped_lock<std::mutex>(queueMutex);
                commandQueue.push_back(cmd);
            }
        }
    });
}

std::string HttpService::ListeningInterface()
{
    return listeningInterface;
}

bool HttpService::ServerRunning()
{
    return srv->is_running();
}

HttpCommand HttpService::PopCommand()
{
    auto lock = std::scoped_lock<std::mutex>(queueMutex);
    if (commandQueue.size() > 0)
    {
        auto cmd = commandQueue.back();
        commandQueue.pop_back();
        return cmd;
    }
    return { CommandType::None };
}
