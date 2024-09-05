#pragma once

#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>
#include <GBA/include/System/EventScheduler.hpp>
#include <GBA/include/Types/DebugTypes.hpp>
#include <GBA/include/Types/Types.hpp>

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

    /// @brief Log the current instruction about to be executed and the registers state before execution.
    /// @param instruction Disassembled instruction info.
    /// @param regState Current value of CPU registers.
    void LogCPU(debug::cpu::DisassembledInstruction const& instruction, debug::cpu::RegState const& regState);

    /// @brief Log when an exception occurs.
    /// @param error Reference to exception that was thrown.
    void LogException(std::exception const& error);

    /// @brief Log when an interrupt request occurs.
    /// @param interrupt Type of interrupt being requested.
    /// @param IE IE register.
    /// @param IME IME status.
    void LogInterruptRequest(u16 interrupt, u16 IE, bool IME);

    /// @brief Log when a halt or unhalt occurs.
    /// @param halted True if system is being halted, false if unhalted.
    /// @param IE IE register.
    /// @param IF IF register.
    void LogHalt(bool halted, u16 IE, u16 IF);

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
