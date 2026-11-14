#include <httplib.h>   // first, before any Qt header

#include "ApiServerBase.h"

#include "ApiJson.h"

ApiServerBase::ApiServerBase(Options options) : m_options(std::move(options)) {}

ApiServerBase::~ApiServerBase() { stop(); }

bool ApiServerBase::start(QString* error) {
    if (isRunning()) return true;

    m_server = std::make_unique<httplib::Server>();
    registerRoutes(*m_server);

    if (m_options.port == 0) {
        m_port = m_server->bind_to_any_port(m_options.host);
    } else {
        m_port = m_server->bind_to_port(m_options.host, m_options.port) ? m_options.port : -1;
    }
    if (m_port < 0) {
        if (error) {
            *error = QStringLiteral("cannot bind %1:%2")
                         .arg(QString::fromStdString(m_options.host))
                         .arg(m_options.port);
        }
        m_server.reset();
        return false;
    }

    m_thread = std::thread([this] { m_server->listen_after_bind(); });
    return true;
}

void ApiServerBase::stop() {
    if (m_server) m_server->stop();
    if (m_thread.joinable()) m_thread.join();
}

void ApiServerBase::reply(httplib::Response& res, int status, const QJsonObject& body) {
    res.status = status;
    res.set_content(ApiJson::dump(body), "application/json");
}
