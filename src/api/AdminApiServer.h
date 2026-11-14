#pragma once
#include "ApiServerBase.h"
#include "ConfigService.h"

// Admin REST API: configuration only. Bind to localhost; every request needs X-Admin-Token.
class AdminApiServer : public ApiServerBase {
public:
    AdminApiServer(ConfigService& config, Options options);
    ~AdminApiServer() override;

protected:
    void registerRoutes(httplib::Server& server) override;

private:
    QJsonObject configBody(const ConfigResult* result = nullptr) const;
    ConfigService& m_config;
};
