#pragma once

#include <fstream>
#include <iomanip>
#include <string_view>

namespace core
{
    class ClientLogger
    {
        class Sequence
        {
            template <typename Char, typename Traits>
            friend std::basic_ostream<Char, Traits>& operator<<(
                std::basic_ostream<Char, Traits>& os, Sequence& seq);

            std::size_t m_sequenceNo = 1;
        };

        template <typename Char, typename Traits>
        friend std::basic_ostream<Char, Traits> &operator<<(
            std::basic_ostream<Char, Traits> &os, Sequence& seq);

    public:

        ClientLogger(std::string_view prefix, std::string_view name);
        ~ClientLogger();

        ClientLogger(const ClientLogger&) = delete;
        ClientLogger(ClientLogger&&) noexcept = delete;

        ClientLogger& operator=(const ClientLogger&) = delete;
        ClientLogger& operator=(ClientLogger&&) noexcept = delete;

        void putRecord(std::string_view record);
        std::ofstream openFile(std::string_view prefix, std::string_view name);

    private:

        Sequence m_sequence;
        std::ofstream m_file;

    };

    template <typename Char, typename Traits>
    std::basic_ostream<Char, Traits> &operator<<(std::basic_ostream<Char, Traits> &os, ClientLogger::Sequence& seq)
    {
        os << '[' << std::setw(4) << std::setfill(Char('0'))
           << seq.m_sequenceNo++ << "] ";
        return os;
    }
}
