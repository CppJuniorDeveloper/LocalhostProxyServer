
#include "ProxyServer.hpp"
#include "ProxySession.hpp"

#include "Logger.hpp"

#include <functional>
#include <iostream>

core::ProxyServer::ProxyServer(tcp::endpoint endpoint, tcp::endpoint target_endpoint)
    : m_acc(m_ioContext, endpoint),
    m_endpoint(std::move(endpoint)),
    m_target_endpoint(std::move(target_endpoint))
{}

core::ProxyServer::~ProxyServer()
{
    m_ioContext.stop();
    if (m_thread.joinable())
    {
        m_thread.join();
    }
    m_isRunning.store(false);
    utils::logger::log_info("ProxyServer stopped.");
}

void core::ProxyServer::run()
{
    if (!m_isRunning)
    {
        m_isRunning.store(true);
        utils::logger::log_info("ProxyServer starts running | ip: {} | port: {} | ...", m_endpoint.address().to_string(), m_endpoint.port());
        m_thread = std::thread(std::bind_front(&ProxyServer::m_launchServer, this));
    }
}

bool core::ProxyServer::checkIsRunning() const noexcept
{
    return m_isRunning.load();
}

void core::ProxyServer::wait() const noexcept
{
    while (this->checkIsRunning())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void core::ProxyServer::m_launchServer()
{
    try
    {
        asio::co_spawn(this->m_ioContext, std::bind_front(&ProxyServer::m_listen, this), [](std::exception_ptr e_ptr)
        {
            if (e_ptr) std::rethrow_exception(e_ptr);
        });

        asio::co_spawn(this->m_ioContext, std::bind_front(&ProxyServer::m_pingTarget, this, 1u), asio::detached);

        this->m_ioContext.run();
        this->m_isRunning.store(false);
    }
    catch (const std::exception& e)
    {
        utils::logger::log_fatal("ProxyServer fatal error: {}.", e.what());
        m_isRunning.store(false);
    }
}

asio::awaitable<void> core::ProxyServer::m_listen()
{
    for(;;)
    {
        auto client_socket = co_await m_acc.async_accept(asio::use_awaitable);
        auto session = ProxySession::createSession(std::move(client_socket), this->m_target_endpoint);
        asio::co_spawn(this->m_ioContext, std::bind_front(&ProxyServer::m_handleSession, this, session), [](std::exception_ptr e_ptr)
        {
            if (e_ptr)
            {
                try
                {
                    std::rethrow_exception(e_ptr);
                }
                catch (const std::exception& e)
                {
                    utils::logger::log_error("Session error: {}", e.what());
                }
            }
        });
    }
    co_return;
}

asio::awaitable<void> core::ProxyServer::m_handleSession(std::shared_ptr<ProxySession> proxy_session)
{
    co_await proxy_session->forwardTraffic();
    co_await proxy_session->waitForClose();

    co_return;
}

asio::awaitable<void> core::ProxyServer::m_pingTarget(unsigned int time_out)
{
    static constexpr unsigned int FAILURE_THRESHOLD = 3u;

    asio::steady_timer timer(this->m_ioContext);
    tcp::socket test_socket(this->m_ioContext);

    unsigned int consecutive_failures{};
    error_code ec{};

    while (m_isRunning.load())
    {
        if (consecutive_failures == FAILURE_THRESHOLD)
        {
            utils::logger::log_fatal("Target server is down. Stopping proxy.");
            this->m_isRunning.store(false);
            co_return;
        }

        timer.expires_after(std::chrono::seconds(time_out));
        co_await timer.async_wait(asio::use_awaitable);

        co_await test_socket.async_connect(this->m_target_endpoint, asio::redirect_error(asio::use_awaitable, ec));

        if (ec)
        {
            utils::logger::log_warning("Target server is unreachable.");
            ++consecutive_failures;
        }
        else
        {
            utils::logger::log_debug("Target server is reachable.");
            consecutive_failures = {};
        }

        test_socket.close();
    }
}
