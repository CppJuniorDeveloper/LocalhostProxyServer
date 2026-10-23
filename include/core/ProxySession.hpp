#pragma once

#include "BoostHeaders.hpp"

namespace asio = boost::asio;
using asio::ip::tcp;
using asio::experimental::channel;
using boost::system::error_code;

namespace core
{
    class ProxySession : public std::enable_shared_from_this<ProxySession>
    {
    public:

        template<typename... Args>
        static std::shared_ptr<ProxySession> createSession(Args&&... args)
        {
            return std::shared_ptr<ProxySession>(new ProxySession(std::forward<Args>(args)...));
        }

        static bool isDisconnectError(const error_code& ec) noexcept;

    public:

        ~ProxySession();

    public:

        asio::awaitable<void> forwardTraffic();
        asio::awaitable<void> waitForClose();

        uint32_t getId() const noexcept;

    private:

        ProxySession(tcp::socket client_socket, tcp::endpoint target_endpoint);

        ProxySession(const ProxySession&) = delete;
        ProxySession& operator=(const ProxySession&) = delete;

    private:

        asio::awaitable<void> m_pump(tcp::socket& from, tcp::socket& to);
        void m_closeSession();

    private:

        static inline uint32_t m_SessionIdCounter{};

        tcp::socket m_clientSocket;
        tcp::socket m_targetSocket;
        tcp::endpoint m_targetEndpoint;

        channel<void(error_code)> m_closeSignal;

        uint32_t m_id;

        bool m_isSessionOpen{};

    };
}
