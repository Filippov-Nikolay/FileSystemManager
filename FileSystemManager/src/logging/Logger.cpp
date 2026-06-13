#include "Logger.h"

#include <chrono>
#include <iomanip>

Logger::Logger(const std::string& filePath)
    : file(filePath, std::ios::app)
{
}

void Logger::Info(const std::string& message)
{
    Write("INFO", message);
}

void Logger::Error(const std::string& message)
{
    Write("ERROR", message);
}

void Logger::Write(const std::string& level, const std::string& message)
{
    if (!file.is_open())
    {
        return;
    }

    const auto now = std::chrono::system_clock::now();
    const auto time = std::chrono::system_clock::to_time_t(now);

    std::tm localTime{};

#ifdef _WIN32
    localtime_s(&localTime, &time);
#else
    localtime_r(&time, &localTime);
#endif

    file << "[" << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S") << "] "
         << "[" << level << "] "
         << message << '\n';

    file.flush();
}
