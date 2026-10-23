#pragma once

#include "boost/algorithm/string/trim_all.hpp"
#include "boost/asio/ip/tcp.hpp"

#include <charconv>

namespace asio = boost::asio;
using asio::ip::tcp;

namespace utils::parse
{
    uint16_t parse_port(std::string_view port_str)
    {
        uint16_t port{};

        auto [ptr, ec] = std::from_chars(port_str.data(), port_str.data() + port_str.size(), port);

        if (ec != std::errc())
        {
            throw std::invalid_argument("Invalid port_str: '" + std::string(port_str) + "'");
        }
        if (ptr != port_str.data() + port_str.size())
        {
            throw std::invalid_argument("Port contains garbage: '" + std::string(port_str) + "'");
        }

        return port;
    }

    tcp::endpoint make_endpoint(std::string_view ip, std::string_view port_str)
    {
        boost::system::error_code ec{};

        asio::ip::address ip_address = asio::ip::make_address(boost::algorithm::trim_copy(ip), ec);

        if (ec)
        {
            throw boost::system::system_error(ec, "Parsing an ip address");
        }

        boost::asio::ip::port_type port = parse_port(boost::algorithm::trim_copy(port_str));

        return tcp::endpoint{ip_address, port};
    }
}
