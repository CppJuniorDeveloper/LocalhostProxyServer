#include "ClientLogger.hpp"

#include <boost/algorithm/string/trim_all.hpp>

#include <cassert>
#include <chrono>
#include <filesystem>
#include <sstream>
#include <thread>

std::ofstream core::ClientLogger::openFile(std::string_view prefix, std::string_view name)
{
    namespace fs = std::filesystem;

    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    now.time_since_epoch()) % 1000;

    std::ostringstream filename;
    filename << prefix << '_' << name << '_'
                << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S")
                << '_' << std::setw(3) << std::setfill('0') << ms.count()
                << ".log";

    fs::path filepath = fs::path("logs") / filename.str();
    
    if (filepath.has_parent_path())
    {
        fs::create_directories(filepath.parent_path());
    }

    std::ofstream file(filepath, std::ios::out | std::ios::app);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file: " + filepath.string());
    }
    return file;
}

core::ClientLogger::ClientLogger(std::string_view prefix, std::string_view name) : m_file{openFile(prefix, name)}
{
    m_file << m_sequence << "---=== Logger \"" << name << "\" started ===---" << std::endl;
}

core::ClientLogger::~ClientLogger()
{
    try
    {
        m_file << m_sequence << "^^^ ^^^ ^^^ Logger stopped ^^^ ^^^ ^^^" << std::endl;
    }
    catch(...)
    {
        assert(false);
    }

}

void core::ClientLogger::putRecord(std::string_view record)
{
    m_file << m_sequence << boost::algorithm::trim_copy(record) << std::endl;
}
