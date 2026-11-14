#pragma once
#include <memory>
#include <string>
#include <thread>
#include <QString>
#include "TrainerService.h"

namespace httplib { class Server; }   // httplib.h stays out of our headers

// Client REST API. Runs its own listening thread; handlers call the thread-safe facade.
class ClientApiServer {
public:
    struct Options {
        std::string host = "127.0.0.1";
        int port = 8080;               // 0 = any free port (tests)
    };

    ClientApiServer(TrainerService& service, Options options);
    ~ClientApiServer();
    ClientApiServer(const ClientApiServer&) = delete;
    ClientApiServer& operator=(const ClientApiServer&) = delete;

    bool start(QString* error = nullptr);
    void stop();                       // safe to call more than once
    bool isRunning() const { return m_thread.joinable(); }
    int port() const { return m_port; }

private:
    void registerRoutes();

    TrainerService& m_service;
    Options m_options;
    std::unique_ptr<httplib::Server> m_server;
    std::thread m_thread;
    int m_port = -1;
};
