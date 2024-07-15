#include <GBA/include/Logging/Logger.hpp>
#include <chrono>
#include <format>
#include <fstream>
#include <string>
#include <GBA/include/System/EventScheduler.hpp>

namespace logging
{
Logger::Logger(fs::path logDir, EventScheduler const& scheduler) : scheduler_(scheduler)
{
    initialized_ = false;
    logPath_ = "";

    if (logDir.empty() || !fs::is_directory(logDir))
    {
        return;
    }

    auto now = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now());
    fs::path logName = std::format("{0:%F}_{0:%H}{0:%M}{0:%S}", now);
    fs::path logFilePath = logDir / logName;
    logFilePath.replace_extension(".log");
    logPath_ = logFilePath;
    buffer_.reserve(LOG_BUFFER_SIZE);
    initialized_ = true;
}

void Logger::LogCPU(std::string instruction, std::string state)
{
    if (!initialized_)
    {
        return;
    }

    std::string cpuMessage = std::format("{:45}", instruction) + state;
    AddToLog(cpuMessage);
}

void Logger::LogException(std::exception const& error)
{
    if (!initialized_)
    {
        return;
    }

    AddToLog(error.what());
}

void Logger::DumpRemainingBuffer()
{
    if (!initialized_)
    {
        return;
    }

    DumpBuffer(true);
}

void Logger::AddToLog(std::string message)
{
    if (buffer_.size() == LOG_BUFFER_SIZE)
    {
        DumpBuffer(false);
    }

    std::string logEntry = std::format("{0:010}  -  ", scheduler_.GetTotalElapsedCycles()) + message;
    buffer_.push_back(logEntry);
}

void Logger::DumpBuffer(bool append)
{
    std::ofstream log;

    if (append)
    {
        log.open(logPath_, std::ios_base::app);
    }
    else
    {
        log.open(logPath_);
    }

    for (std::string const& message : buffer_)
    {
        log << message << "\n";
    }

    buffer_.clear();
}
}  // namespace logging
