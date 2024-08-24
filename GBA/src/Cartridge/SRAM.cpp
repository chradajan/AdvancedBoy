#include <GBA/include/Cartridge/SRAM.hpp>
#include <array>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <GBA/include/Memory/MemoryMap.hpp>
#include <GBA/include/System/SystemControl.hpp>
#include <GBA/include/Types.hpp>
#include <GBA/include/Utilities/CommonUtils.hpp>

namespace cartridge
{
SRAM::SRAM(fs::path savePath, SystemControl& systemControl) : systemControl_(systemControl)
{
    savePath_ = savePath;
    sram_.fill(std::byte{0xFF});

    if (fs::exists(savePath) && (fs::file_size(savePath) == sram_.size()))
    {
        std::ifstream saveFile(savePath, std::ios::binary);

        if (!saveFile.fail())
        {
            saveFile.read(reinterpret_cast<char*>(sram_.data()), sram_.size());
        }
    }
}

bool SRAM::IsBackupMediaAccess(u32 addr) const
{
    return (SRAM_ADDR_MIN <= addr) && (addr <= SRAM_ADDR_MAX);
}

MemReadData SRAM::ReadMem(u32 addr, AccessSize length)
{
    if (addr >= SRAM_ADDR_MIN + sram_.size())
    {
        addr = SRAM_ADDR_MIN + (addr % sram_.size());
    }

    int cycles = 1 + systemControl_.WaitStates(WaitStateRegion::SRAM, false, length);
    u32 val = ReadMemoryBlock(sram_, addr, SRAM_ADDR_MIN, AccessSize::BYTE);

    if (length != AccessSize::BYTE)
    {
        val = Read8BitBus(val, length);
    }

    return {cycles, val, false};
}

int SRAM::WriteMem(u32 addr, u32 val, AccessSize length)
{
    if (addr >= SRAM_ADDR_MIN + sram_.size())
    {
        addr = SRAM_ADDR_MIN + (addr % sram_.size());
    }

    int cycles = 1 + systemControl_.WaitStates(WaitStateRegion::SRAM, false, length);

    if (length != AccessSize::BYTE)
    {
        val = Write8BitBus(addr, val);
    }

    WriteMemoryBlock(sram_, addr, SRAM_ADDR_MIN, val, AccessSize::BYTE);
    return cycles;
}

void SRAM::Save() const
{
    std::ofstream saveFile(savePath_, std::ios::binary);

    if (!saveFile.fail())
    {
        saveFile.write(reinterpret_cast<const char*>(sram_.data()), sram_.size());
    }
}
}  // namespace cartridge
