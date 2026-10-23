#include "ProxyServer.hpp"
#include "ServerProperties.hpp"

#include "Parse.hpp"
#include "Logger.hpp"

int main(int argc, const char* argv[])
{
    utils::logger::global_log_min_level = utils::logger::LogLevel::INFO;

    std::string_view programm_name = argv[0];

    if (argc != 3)
    {
        utils::logger::log_error("Usage: {} <target_ip> <target_port>", programm_name);
        utils::logger::log_error("target_ip and target_port specify the server where all traffic will be forwarded through the proxy.");
        utils::logger::log_error("Example: {} 127.0.0.1 8080", programm_name);
        utils::logger::log_debug("{}", argc);
        return EXIT_FAILURE;
    }

    std::string_view target_ip = argv[1];
    std::string_view target_port = argv[2];

    utils::logger::log_info("Target ip: {} | Target port: {}", target_ip, target_port);

    try
    {
        auto target_endpoint = utils::parse::make_endpoint(target_ip, target_port);

        core::ProxyServer proxy{tcp::endpoint{tcp::v4(), proxy_server_properties::PROXY_PORT}, target_endpoint};

        proxy.run();

        proxy.wait();

        return EXIT_SUCCESS;
    }
    catch(const std::exception& ex)
    {
        utils::logger::log_fatal("ProxyServer crashed with a fatal error: {}", ex.what());
    }

    return EXIT_FAILURE;
}
