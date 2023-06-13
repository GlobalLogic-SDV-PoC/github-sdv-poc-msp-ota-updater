#include "updater/app.hpp"

#include "ipc/client.hpp"
#include "ipc/util.hpp"
#include "spdlog/fmt/fmt.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/spdlog.h"
#include "updater/fs.hpp"
#ifdef __ANDROID__
#include "spdlog/sinks/android_sink.h"
#else
#include "spdlog/sinks/stdout_color_sinks.h"
#endif

namespace updater
{
App::App(const std::shared_ptr<IUpdater>& updater, nlohmann::json config)
    : m_config(std::move(config))
    , m_updater(updater)
{
}

void App::init()
{
    SPDLOG_DEBUG("[updater] App initialization: starting...");
    initIpc();
    SPDLOG_DEBUG("[updater] App initialization: done.");
}

void App::initIpc()
{
    auto& ipc_config = m_config["ipc"];
    ipc::Client::Config client_config;
    client_config.host = ipc_config["host"];
    client_config.service = ipc_config["service"];
    client_config.header_buffer_size = ipc_config["header_buffer_size"];
    client_config.body_buffer_size = ipc_config["body_buffer_size"];
    client_config.on_receive_handler = ipc::bind_front(&App::onIpcReceived, this);
    client_config.on_connected_handler = std::bind(&App::onIpcConnected, this);

    m_ipc_client = std::make_shared<ipc::Client>(m_context, std::move(client_config));
}

void App::start()
{
    SPDLOG_DEBUG("[updater] Starting app.");
    m_ipc_client->start();
    m_context.run();
}
void App::stop()
{
    m_ipc_client->stop();
}
void App::initDefaultLogger(std::string_view filepath,
                            size_t max_size,
                            size_t max_files,
                            std::chrono::seconds flush_interval)
{
#ifdef __ANDROID__
    auto console_logger = std::make_shared<spdlog::sinks::android_sink_mt>("updater");
#else
    auto console_logger = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
#endif
    console_logger->set_level(spdlog::level::trace);

    auto file_logger
        = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(filepath.data(),
                                                                 max_size,
                                                                 max_files);
    file_logger->set_level(spdlog::level::trace);

    m_log_dir = fs::absolute(
        fs::path(
            file_logger->filename())
            .parent_path());

    auto new_logger = std::make_shared<spdlog::logger>("file_console",
                                                       spdlog::sinks_init_list{std::move(file_logger),
                                                                               std::move(console_logger)});

    spdlog::set_default_logger(std::move(new_logger));
    spdlog::set_pattern("[%H:%M:%S.%e] [%^%l%$] %v");
    spdlog::set_level(static_cast<spdlog::level::level_enum>(SPDLOG_ACTIVE_LEVEL));
    spdlog::flush_every(flush_interval);
}

void App::onIpcConnected()
{
    auto packet = std::make_shared<ipc::Packet>();
    packet->header["action"] = "subscribe";
    packet->header["topic"] = m_config["update_topic"];
    m_ipc_client->post(packet);
}
void App::onIpcReceived(const std::shared_ptr<ipc::Packet>& packet)
{
    SPDLOG_DEBUG("[updater] IPC received.");
    if (packet->header["action"] != "forward")
    {
        return;
    }
    auto download_config = nlohmann::json::parse(packet->payload);
    SPDLOG_DEBUG("[updater] Download image: download url {}", download_config["bucketUrl"]);
    const auto downloader = fmt::format("wget -O {} \"{}\"", m_config["download_path"], download_config["bucketUrl"]);
    std::system(downloader.c_str());
    SPDLOG_DEBUG("[updater] Download image: done.");
    // file is downloaded
    m_updater->update(m_config["download_path"].get<std::string>());
}
}  // namespace updater