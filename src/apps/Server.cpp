
#include "BoostHeaders.hpp"

#include "ClientLogger.hpp"
#include "ProxyServer.hpp"

#include "Logger.hpp"
#include "ServerProperties.hpp"

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

namespace
{
    bool isDisconnectError(const boost::system::error_code& ec)
    {
        return ec == asio::error::eof
            || ec == asio::error::connection_reset
            || ec == asio::error::operation_aborted
            || ec == asio::error::broken_pipe
            || ec == asio::error::connection_aborted;
    }

    asio::awaitable<void> worksWithClient(tcp::socket socket)
    {
        asio::streambuf buffer;
        boost::system::error_code ec{};

        co_await asio::async_read_until(socket, buffer, "\n", asio::redirect_error(asio::use_awaitable, ec));

        if (ec)
        {
            // utils::logger::log_error("Connection error: {} #{}.", ec.category().name(), ec.value());
            co_return;
        }

        std::string client_name{asio::buffers_begin(buffer.data()), asio::buffers_end(buffer.data())};
        boost::trim(client_name);

        core::ClientLogger client_logger{"co", client_name};
        std::string response = "Data successfully received.";

        buffer.consume(buffer.size());

        utils::logger::log_info("Client [{}] connected.", client_name);

        auto trimLF = [](const std::string& str) -> std::string
        {
            std::string trimmed{str};

            while (!trimmed.empty() && trimmed.back() == '\n')
            {
                trimmed.pop_back();
            }

            return trimmed;
        };

        for (;;)
        {
            buffer.consume(buffer.size());

            co_await asio::async_read_until(socket, buffer, "\n", asio::redirect_error(asio::use_awaitable, ec));

            if (isDisconnectError(ec))
            {
                utils::logger::log_info("Client [{}] disconnected.", client_name);
                co_return;
            }

            if (ec)
            {
                utils::logger::log_error("[{}] | Read error: {} #{}.", client_name, ec.category().name(), ec.value());
                co_return;
            }

            std::string log_record{asio::buffers_begin(buffer.data()), asio::buffers_end(buffer.data())};

            utils::logger::log_info("{} sent message \"{}\".", client_name, trimLF(log_record));

            client_logger.putRecord(log_record);

            co_await asio::async_write(socket, asio::buffer(response + "\n"), asio::redirect_error(asio::use_awaitable, ec));

            if (ec)
            {
                if (isDisconnectError(ec))
                {
                    utils::logger::log_info("Сlient [{}] disconnected during write.", client_name);
                }
                else
                {
                    utils::logger::log_error("[{}] | Write error: {} #{}.", client_name, ec.category().name(), ec.value());
                }
            }
        }
    }

    asio::awaitable<void> listen()
    {
        const auto executor = co_await asio::this_coro::executor;

        utils::logger::log_info("Listening TCP v4 port {} for new log clients...", server_properties::PORT);
        tcp::acceptor acceptor{executor, {tcp::v4(), server_properties::PORT}};

        for (;;)
        {
            tcp::socket socket = co_await acceptor.async_accept(asio::use_awaitable);
            utils::logger::log_debug("Accepting new connection...");

            asio::co_spawn(executor, worksWithClient(std::move(socket)), asio::detached);
        }
    }

    void runServer()
    {
        utils::logger::log_info("Server running...");

        asio::io_context ioContext;

        asio::co_spawn(ioContext, listen, asio::detached);

        ioContext.run();

        utils::logger::log_info("Server stopped.");
    }
}

auto main() -> int
{
    utils::logger::global_log_min_level = utils::logger::LogLevel::INFO;

    try
    {
        runServer();
        return EXIT_SUCCESS;
    }
    catch(const std::exception& ex)
    {
        utils::logger::log_fatal("Fatal error \"{}\".", ex.what());
    }
    catch (...)
    {
        utils::logger::log_fatal("Fatal UNKNOWN error.");
    }

    return EXIT_FAILURE;
}
