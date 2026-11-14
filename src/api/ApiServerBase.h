#pragma once
#include <memory>
#include <string>
#include <thread>
#include <QJsonObject>
#include <QString>

namespace httplib {
class Server;
struct Response;
}

// Owns an httplib::Server and its listening thread; subclasses add the routes.
// A subclass destructor must call stop(), so no handler runs while it is destroyed.
class ApiServerBase {
public:
    struct Options {
        std::string host = "127.0.0.1";
        int port = 0;   // 0 = any free port
    };

    explicit ApiServerBase(Options options);
    virtual ~ApiServerBase();
    ApiServerBase(const ApiServerBase&) = delete;
    ApiServerBase& operator=(const ApiServerBase&) = delete;

    bool start(QString* error = nullptr);
    void stop();   // safe to call more than once
    bool isRunning() const { return m_thread.joinable(); }
    int port() const { return m_port; }

protected:
    virtual void registerRoutes(httplib::Server& server) = 0;
    static void reply(httplib::Response& res, int status, const QJsonObject& body);

private:
    Options m_options;
    std::unique_ptr<httplib::Server> m_server;
    std::thread m_thread;
    int m_port = -1;
};
