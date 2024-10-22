#include <GBA/include/Cartridge/EEPROM.hpp>
#include <filesystem>
#include <fstream>
#include <utility>
#include <vector>
#include <GBA/include/Memory/MemoryMap.hpp>
#include <GBA/include/System/SystemControl.hpp>
#include <GBA/include/Utilities/CommonUtils.hpp>
#include <GBA/include/Utilities/Types.hpp>

namespace cartridge
{
EEPROM::EEPROM(fs::path savePath, bool largeCart, SystemControl& systemControl) :
    largeCart_(largeCart),
    systemControl_(systemControl)
{
    savePath_ = savePath;
    readIndex_ = U16_MAX;

    if (fs::exists(savePath))
    {
        size_t fileSize = fs::file_size(savePath);

        if ((fileSize == 512) || (fileSize == (8 * KiB)))
        {
            std::ifstream saveFile(savePath, std::ios::binary);

            if (!saveFile.fail())
            {
                eeprom_.resize(fileSize / sizeof(u64));
                saveFile.read(reinterpret_cast<char*>(eeprom_.data()), fileSize);
            }
        }
    }
}

bool EEPROM::IsBackupMediaAccess(u32 addr) const
{
    if (largeCart_)
    {
        return ((EEPROM_LARGE_CART_ADDR_MIN <= addr) && (addr <= EEPROM_ADDR_MAX));
    }

    return ((EEPROM_SMALL_CART_ADDR_MIN <= addr) && (addr <= EEPROM_ADDR_MAX));
}

MemReadData EEPROM::ReadMem(u32 addr, AccessSize length)
{
    (void)addr;
    int cycles = 1 + systemControl_.WaitStates(WaitStateRegion::TWO, false, length);
    return {cycles, 1, false};
}

int EEPROM::WriteMem(u32 addr, u32 val, AccessSize length)
{
    (void)addr;
    (void)val;
    return 1 + systemControl_.WaitStates(WaitStateRegion::TWO, false, length);
}

int EEPROM::SetIndex(u16 index, u8 indexSize)
{
    int cycles = indexSize + 3;
    cycles += systemControl_.WaitStates(WaitStateRegion::TWO, false, AccessSize::HALFWORD) +
              (systemControl_.WaitStates(WaitStateRegion::TWO, true, AccessSize::HALFWORD) * (indexSize + 2));

    if (eeprom_.empty())
    {
        if (indexSize == 6)
        {
            eeprom_.resize(64, U64_MAX);
        }
        else if (indexSize == 14)
        {
            eeprom_.resize(1024, U64_MAX);
        }
    }

    readIndex_ = index & 0x03FF;
    return cycles;
}

std::pair<u64, int> EEPROM::ReadDWord()
{
    int cycles = 68;
    cycles += systemControl_.WaitStates(WaitStateRegion::TWO, false, AccessSize::HALFWORD) +
              (systemControl_.WaitStates(WaitStateRegion::TWO, true, AccessSize::HALFWORD) * 67);

    u64 val;

    if (eeprom_.empty() || (readIndex_ >= eeprom_.size()))
    {
        val = U64_MAX;
    }
    else
    {
        val = eeprom_[readIndex_];
    }

    return {val, cycles};
}

int EEPROM::WriteDWord(u16 index, u8 indexSize, u64 val)
{
    int cycles = 67 + indexSize;
    cycles += systemControl_.WaitStates(WaitStateRegion::TWO, false, AccessSize::HALFWORD) +
              (systemControl_.WaitStates(WaitStateRegion::TWO, true, AccessSize::HALFWORD) * (indexSize + 66));

    if (eeprom_.empty())
    {
        if (indexSize == 6)
        {
            eeprom_.resize(64, U64_MAX);
        }
        else if (indexSize == 14)
        {
            eeprom_.resize(1024, U64_MAX);
        }
    }

    index &= 0x03FF;

    if (!eeprom_.empty() && (index < eeprom_.size()))
    {
        eeprom_[index] = val;
    }

    return cycles;
}

void EEPROM::Save() const
{
    if (!eeprom_.empty())
    {
        std::ofstream saveFile(savePath_, std::ios::binary);

        if (!saveFile.fail())
        {
            saveFile.write(reinterpret_cast<const char*>(eeprom_.data()), eeprom_.size() * sizeof(u64));
        }
    }
}

void EEPROM::Serialize(std::ofstream& saveState) const
{
    size_t size = eeprom_.size();
    SerializeTrivialType(size);
    SerializeArray(eeprom_);
    SerializeTrivialType(readIndex_);
}

void EEPROM::Deserialize(std::ifstream& saveState)
{
    size_t size;
    DeserializeTrivialType(size);
    eeprom_.resize(size);
    DeserializeArray(eeprom_);
    DeserializeTrivialType(readIndex_);
}
}  // namespace cartridge
