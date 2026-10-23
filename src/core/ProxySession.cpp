#include "ProxySession.hpp"

#include "Logger.hpp"

core::ProxySession::ProxySession(tcp::socket client_socket, tcp::endpoint target_endpoint)
    : m_clientSocket(std::move(client_socket)),
      m_targetSocket(m_clientSocket.get_executor()),
      m_targetEndpoint(std::move(target_endpoint)),
      m_closeSignal(m_clientSocket.get_executor(), 1)
{
    m_isSessionOpen = true;
    ++m_SessionIdCounter;
    m_id = m_SessionIdCounter;
}

core::ProxySession::~ProxySession()
{
    if (m_isSessionOpen)
    {
        this->m_closeSession();
    }
}

bool core::ProxySession::isDisconnectError(const error_code &ec) noexcept
{
    return ec == asio::error::eof
        || ec == asio::error::connection_reset
        || ec == asio::error::operation_aborted
        || ec == asio::error::broken_pipe
        || ec == asio::error::connection_aborted;
}

asio::awaitable<void> core::ProxySession::forwardTraffic()
{
    auto executor = m_clientSocket.get_executor();
    auto self = shared_from_this();

    error_code ec{};
    co_await m_targetSocket.async_connect(m_targetEndpoint, asio::redirect_error(asio::use_awaitable, ec));

    if (ec)
    {
        std::string unreachable_server_response{"Target server is unreachable"};

        utils::logger::log_error("Cannot connect to target: {}", ec.message());

        co_await asio::async_write(this->m_clientSocket, asio::buffer(unreachable_server_response + "\n"), asio::redirect_error(asio::use_awaitable, ec));

        this->m_closeSession();
        co_return;
    }

    asio::co_spawn(executor,
    [self]() -> asio::awaitable<void>
    {
        co_await self->m_pump(self->m_clientSocket, self->m_targetSocket);
    },
    [self](std::exception_ptr e_ptr)
    {
        if (e_ptr)
        {
            try
            {
                std::rethrow_exception(e_ptr);
            }
            catch (const std::exception& e)
            {
                utils::logger::log_error("Session [ID: {}] pump (client -> target) crashed: {}",
                    self->m_id, e.what());
            }
        }
    });

    asio::co_spawn(executor,
    [self]() -> asio::awaitable<void>
    {
        co_await self->m_pump(self->m_targetSocket, self->m_clientSocket);
    },
    [self](std::exception_ptr e_ptr)
    {
        if (e_ptr)
        {
            try
            {
                std::rethrow_exception(e_ptr);
            }
            catch (const std::exception& e)
            {
                utils::logger::log_error("Session [ID: {}] pump (target -> client) crashed: {}",
                    self->m_id, e.what());
            }
        }
    });

    utils::logger::log_info("Session [ID: {}] started (Target: {}:{})",
        m_id,
        m_targetEndpoint.address().to_string(),
        m_targetEndpoint.port()
    );
}

asio::awaitable<void> core::ProxySession::waitForClose()
{
    co_await m_closeSignal.async_receive(asio::use_awaitable);
}

uint32_t core::ProxySession::getId() const noexcept
{
    return m_id;
}

asio::awaitable<void> core::ProxySession::m_pump(tcp::socket& from, tcp::socket& to)
{
    asio::streambuf buffer{};
    boost::system::error_code ec{};

    for (;;)
    {
        buffer.consume(buffer.size());

        co_await asio::async_read_until(from, buffer, "\n", asio::redirect_error(asio::use_awaitable, ec));

        if (ec)
        {
            if (ProxySession::isDisconnectError(ec))
            {
                error_code shutdown_ec;
                to.shutdown(tcp::socket::shutdown_send, shutdown_ec);
                utils::logger::log_debug("Session [ID: {}] closed by peer (disconnect).", this->m_id);
            }
            else
            {
                utils::logger::log_error("Read error: {} in session [ID: {}]", this->m_id, ec.message());
            }
            break;
        }

        co_await asio::async_write(to, buffer, asio::redirect_error(asio::use_awaitable, ec));

        if (ec)
        {
            if (ProxySession::isDisconnectError(ec))
            {
                utils::logger::log_info("Write failed: peer disconnected in session [ID: {}].", this->m_id);
            }
            else
            {
                utils::logger::log_error("Write error: {} in session [ID: {}]", this->m_id, ec.message());
            }
            break;
        }
    }

    this->m_closeSession();

}

void core::ProxySession::m_closeSession()
{
    if (m_isSessionOpen)
    {
        error_code ec{};
        m_clientSocket.close(ec);
        m_targetSocket.close(ec);
        m_isSessionOpen = false;

        m_closeSignal.try_send(error_code{});

        utils::logger::log_info("Session [ID: {}] closed.", m_id);
    }
}
