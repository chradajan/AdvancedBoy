#pragma once

#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>
#include <GBA/include/System/EventScheduler.hpp>
#include <GBA/include/Types.hpp>

namespace fs = std::filesystem;

namespace logging
{
constexpr size_t LOG_BUFFER_SIZE = 100000;

class Logger
{
public:
    Logger() = delete;
    Logger(Logger const&) = delete;
    Logger& operator=(Logger const&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

    /// @brief Initialize the logger.
    /// @param logDir Path to directory where log file should be generated.
    /// @param scheduler Reference to event scheduler to total cycles elapsed alongside logged events.
    explicit Logger(fs::path logDir, EventScheduler const& scheduler);

    /// @brief Check if logging is currently enabled.
    /// @return Whether logging is enabled.
    bool Enabled() const { return initialized_; }

    /// @brief Log function for logging current CPU state and the instruction about to be executed.
    /// @param instruction Human readable form of instruction about to be executed.
    /// @param state Current register state of CPU before executing instruction.
    /// @param pc Address of the instruction being logged.
    void LogCPU(std::string instruction, std::string state, u32 pc);

    /// @brief Log when an exception occurs.
    /// @param error Reference to exception that was thrown.
    void LogException(std::exception const& error);

    /// @brief Append whatever messages are currently stored in the buffer to the log file.
    void DumpRemainingBuffer();

private:
    /// @brief Add a message to the log along with the current number of CPU cycles that have elapsed since startup.
    /// @param message Message to write to log.
    void AddToLog(std::string message);

    /// @brief Dump log buffer to log file.
    /// @param append Whether to append to log file or overwrite it.
    void DumpBuffer(bool append);

    bool initialized_;
    fs::path logPath_;
    std::vector<std::string> buffer_;

    // External components
    EventScheduler const& scheduler_;
};
}  // namespace logging
