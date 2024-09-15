#include <GBA/include/Logging/Logger.hpp>
#include <chrono>
#include <format>
#include <fstream>
#include <string>
#include <sstream>
#include <GBA/include/CPU/CpuTypes.hpp>
#include <GBA/include/System/EventScheduler.hpp>
#include <GBA/include/System/SystemControl.hpp>
#include <GBA/include/Types/DebugTypes.hpp>
#include <GBA/include/Types/Types.hpp>

namespace
{
std::string InterruptString(InterruptType interrupt)
{
    switch (interrupt)
    {
        case InterruptType::LCD_VBLANK:
            return "LCD_VBLANK";
        case InterruptType::LCD_HBLANK:
            return "LCD_HBLANK";
        case InterruptType::LCD_VCOUNTER_MATCH:
            return "LCD_VCOUNTER_MATCH";
        case InterruptType::TIMER_0_OVERFLOW:
            return "TIMER_0_OVERFLOW";
        case InterruptType::TIMER_1_OVERFLOW:
            return "TIMER_1_OVERFLOW";
        case InterruptType::TIMER_2_OVERFLOW:
            return "TIMER_2_OVERFLOW";
        case InterruptType::TIMER_3_OVERFLOW:
            return "TIMER_3_OVERFLOW";
        case InterruptType::SERIAL_COMMUNICATION:
            return "SERIAL_COMMUNICATION";
        case InterruptType::DMA0:
            return "DMA0";
        case InterruptType::DMA1:
            return "DMA1";
        case InterruptType::DMA2:
            return "DMA2";
        case InterruptType::DMA3:
            return "DMA3";
        case InterruptType::KEYPAD:
            return "KEYPAD";
        case InterruptType::GAME_PAK:
            return "GAME_PAK";
    }

    return "";
}
}  // namespace

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

void Logger::LogCPU(debug::cpu::Mnemonic const& mnemonic, debug::cpu::RegState const& regState, u32 addr, u32 instruction, bool arm)
{
    if (!initialized_)
    {
        return;
    }

    std::stringstream regStream;

    for (u8 i = 0; i < 16; ++i)
    {
        regStream << std::format("R{} {:08X}  ", i, regState.registers[i]);
    }

    regStream << "CPSR: " << (regState.negative ? "N" : "-") <<
                             (regState.zero ? "Z" : "-") <<
                             (regState.carry ? "C" : "-") <<
                             (regState.overflow ? "V" : "-") << "  ";

    regStream << (regState.irqDisable ? "I" : "-") <<
                 (regState.fiqDisable ? "F" : "-") <<
                 (regState.thumbState ? "T" : "-") <<
                 "  " << "Mode: ";

    u32 spsr = regState.spsr.has_value() ? regState.spsr.value() : 0;
    auto mode = static_cast<cpu::OperatingMode>(regState.mode);

    switch (mode)
    {
        case cpu::OperatingMode::User:
            regStream << "User";
            break;
        case cpu::OperatingMode::FIQ:
            regStream << std::format("FIQ         SPSR: {:08X}", spsr);
            break;
        case cpu::OperatingMode::IRQ:
            regStream << std::format("IRQ         SPSR: {:08X}", spsr);
            break;
        case cpu::OperatingMode::Supervisor:
            regStream << std::format("Supervisor  SPSR: {:08X}", spsr);
            break;
        case cpu::OperatingMode::Abort:
            regStream << std::format("Abort       SPSR: {:08X}", spsr);
            break;
        case cpu::OperatingMode::System:
            regStream << "System";
            break;
        case cpu::OperatingMode::Undefined:
            regStream << std::format("Undefined   SPSR: {:08X}", spsr);
            break;
    }

    std::string decodedInstruction = mnemonic.op + mnemonic.cond + " " + mnemonic.args;

    if (arm)
    {
        AddToLog(std::format("{:08X}: {:08X} -> {:30}{}", addr, instruction, decodedInstruction, regStream.str()));
    }
    else
    {
        AddToLog(std::format("{:08X}:     {:04X} -> {:30}{}", addr, instruction, decodedInstruction, regStream.str()));
    }
}

void Logger::LogException(std::exception const& error)
{
    if (!initialized_)
    {
        return;
    }

    AddToLog(error.what());
}

void Logger::LogInterruptRequest(u16 interrupt, u16 IE, bool IME)
{
    std::string message =
        std::format("Requesting {} interrupt. IE: 0x{:04X}, IME: {}", InterruptString(InterruptType{interrupt}), IE, IME);
    AddToLog(message);
}

void Logger::LogHalt(bool halted, u16 IE, u16 IF)
{
    std::string message;

    if (halted)
    {
        message = std::format("Halted. IE: 0x{:04X}, IF: 0x{:04X}", IE, IF);
    }
    else
    {
        auto unhaltReason = static_cast<InterruptType>(IE & IF);
        message = "Unhalted due to " + InterruptString(unhaltReason);
    }

    AddToLog(message);
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
