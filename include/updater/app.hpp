#pragma once

#include "ipc/client.hpp"
#include "nlohmann/json.hpp"
#include "updater/interface/updater.hpp"

namespace updater
{
namespace net = ipc::net;
class App
{
public:
    App(const std::shared_ptr<IUpdater>& updater, nlohmann::json config);
    void init();
    void start();
    void stop();
    void initDefaultLogger(std::string_view filepath,
                           size_t max_size,
                           size_t max_files,
                           std::chrono::seconds flush_interval);

private:
    void initIpc();
    void onIpcReceived(const std::shared_ptr<ipc::Packet>& packet);
    void onIpcConnected();

private:
    net::io_context m_context;
    nlohmann::json m_config;
    std::shared_ptr<IUpdater> m_updater;
    std::shared_ptr<ipc::Client> m_ipc_client;
    std::string m_log_dir;
};
}  // namespace updater