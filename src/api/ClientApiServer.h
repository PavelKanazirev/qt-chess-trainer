#pragma once
#include "ApiServerBase.h"
#include "TrainerService.h"

// Client REST API: puzzles, moves and statistics through the TrainerService facade.
class ClientApiServer : public ApiServerBase {
public:
    ClientApiServer(TrainerService& service, Options options);
    ~ClientApiServer() override;

protected:
    void registerRoutes(httplib::Server& server) override;

private:
    TrainerService& m_service;
};