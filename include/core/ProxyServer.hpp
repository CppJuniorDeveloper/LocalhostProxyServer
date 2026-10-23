#pragma once

#include "BoostHeaders.hpp"

#include <thread>
#include <atomic>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

namespace core
{
    class ProxySession;

    class ProxyServer
    {
    public:

        ProxyServer(tcp::endpoint endpoint, tcp::endpoint target_endpoint);
        ~ProxyServer();

        ProxyServer(const ProxyServer&) = delete;
        ProxyServer& operator=(const ProxyServer&) = delete;

    public:

        void run();
        bool checkIsRunning() const noexcept;
        void wait() const noexcept;

    private:

        void m_launchServer();
        asio::awaitable<void> m_listen();
        asio::awaitable<void> m_handleSession(std::shared_ptr<ProxySession> proxy_session);
        asio::awaitable<void> m_pingTarget(unsigned int time_out);

    private:

        asio::io_context m_ioContext;

        tcp::acceptor m_acc;
        tcp::endpoint m_endpoint;
        tcp::endpoint m_target_endpoint;

        std::thread m_thread;

        std::atomic<bool> m_isRunning{};
    };
}
