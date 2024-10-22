#include <GBA/include/Cartridge/Flash.hpp>
#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <vector>
#include <GBA/include/Memory/MemoryMap.hpp>
#include <GBA/include/System/SystemControl.hpp>
#include <GBA/include/Utilities/CommonUtils.hpp>
#include <GBA/include/Utilities/Types.hpp>

namespace cartridge
{
Flash::Flash(fs::path savePath, bool largeFlash, SystemControl& systemControl) :
    bankedFlash_(largeFlash),
    systemControl_(systemControl)
{
    savePath_ = savePath;

    if (largeFlash)
    {
        flash_.resize(2);
    }
    else
    {
        flash_.resize(1);
    }

    for (auto& bank : flash_)
    {
        bank.fill(std::byte{0xFF});
    }

    if (fs::exists(savePath) && (fs::file_size(savePath) == (largeFlash ? (128 * KiB) : (64 * KiB))))
    {
        std::ifstream saveFile(savePath, std::ios::binary);

        if (!saveFile.fail())
        {
            for (auto& bank : flash_)
            {
                saveFile.read(reinterpret_cast<char*>(bank.data()), bank.size());
            }
        }
    }

    bank_ = 0;
    state_ = FlashState::READY;
    chipIdMode_ = false;
}

bool Flash::IsBackupMediaAccess(u32 addr) const
{
    return (FLASH_ADDR_MIN <= addr) && (addr <= FLASH_ADDR_MAX);
}

MemReadData Flash::ReadMem(u32 addr, AccessSize length)
{
    int cycles = 1 + systemControl_.WaitStates(WaitStateRegion::SRAM, false, length);
    u32 val;

    if (chipIdMode_ && (addr == FLASH_ADDR_MIN))
    {
        val = bankedFlash_ ? 0x62 : 0x32;
    }
    else if (chipIdMode_ && (addr == (FLASH_ADDR_MIN + 1)))
    {
        val = bankedFlash_ ? 0x13 : 0x1B;
    }
    else
    {
        val = ReadMemoryBlock(flash_[bank_], addr, FLASH_ADDR_MIN, AccessSize::BYTE);
    }

    if (length != AccessSize::BYTE)
    {
        val = Read8BitBus(val, length);
    }

    return {cycles, val, false};
}

int Flash::WriteMem(u32 addr, u32 val, AccessSize length)
{
    if (length != AccessSize::BYTE)
    {
        val = Write8BitBus(addr, val);
    }

    u8 byte = val & U8_MAX;
    auto cmd = FlashCommand{byte};
    int cycles = 1 + systemControl_.WaitStates(WaitStateRegion::SRAM, false, length);

    switch (state_)
    {
        case FlashState::READY:
        case FlashState::ERASE_SEQ_READY:
        {
            if ((addr == CMD_ADDR_1) && (cmd == FlashCommand::CMD_SEQ_START))
            {
                state_ = (state_ == FlashState::READY) ? FlashState::CMD_SEQ_STARTED : FlashState::ERASE_SEQ_STARTED;
            }

            break;
        }

        case FlashState::CMD_SEQ_STARTED:
        case FlashState::ERASE_SEQ_STARTED:
        {
            if ((addr == CMD_ADDR_2) && (cmd == FlashCommand::CMD_SEQ_AWAIT))
            {
                state_ = (state_ == FlashState::CMD_SEQ_STARTED) ? FlashState::CMD_SEQ_AWAITING_CMD :
                                                                   FlashState::ERASE_SEQ_AWAITING_CMD;
            }

            break;
        }

        case FlashState::CMD_SEQ_AWAITING_CMD:
        {
            if (addr == CMD_ADDR_1)
            {
                ProcessCommand(cmd);
            }

            break;
        }

        case FlashState::ERASE_SEQ_AWAITING_CMD:
        {
            if ((addr == CMD_ADDR_1) && (cmd == FlashCommand::ERASE_ALL))
            {
                for (auto& bank : flash_)
                {
                    bank.fill(std::byte{0xFF});
                }

                state_ = FlashState::READY;
            }
            else if (cmd == FlashCommand::ERASE_4K_SECTOR)
            {
                u32 block = addr & 0x0000'F000;
                auto blockStart = flash_[bank_].begin() + block;
                auto blockEnd = flash_[bank_].begin() + block + 0x1000;
                std::fill(blockStart, blockEnd, std::byte{0xFF});
                state_ = FlashState::READY;
            }

            break;
        }

        case FlashState::AWAITING_WRITE_DATA:
        {
            WriteMemoryBlock(flash_[bank_], addr, FLASH_ADDR_MIN, byte, AccessSize::BYTE);
            state_ = FlashState::READY;
            break;
        }

        case FlashState::AWAITING_BANK:
        {
            if (addr == FLASH_ADDR_MIN)
            {
                bank_ = byte & 0x01;
                state_ = FlashState::READY;
            }

            break;
        }

        default:
            break;
    }

    return cycles;
}

void Flash::Save() const
{
    std::ofstream saveFile(savePath_, std::ios::binary);

    if (!saveFile.fail())
    {
        for (auto& bank : flash_)
        {
            saveFile.write(reinterpret_cast<const char*>(bank.data()), bank.size());
        }
    }
}

void Flash::ProcessCommand(FlashCommand cmd)
{
    switch (cmd)
    {
        case FlashCommand::ENTER_CHIP_ID_MODE:
            chipIdMode_ = true;
            state_ = FlashState::READY;
            break;
        case FlashCommand::EXIT_CHIP_ID_MODE:
            chipIdMode_ = false;
            state_ = FlashState::READY;
            break;
        case FlashCommand::PREPARE_ERASE:
            state_ = FlashState::ERASE_SEQ_AWAITING_CMD;
            break;
        case FlashCommand::WRITE_DATA:
            state_ = FlashState::AWAITING_WRITE_DATA;
            break;
        case FlashCommand::SET_BANK:
            state_ = bankedFlash_ ? FlashState::AWAITING_BANK : state_;
            break;
        default:
            break;
    }
}

void Flash::Serialize(std::ofstream& saveState) const
{
    for (auto const& bank : flash_)
    {
        SerializeArray(bank);
    }

    SerializeTrivialType(bank_);
    SerializeTrivialType(state_);
    SerializeTrivialType(chipIdMode_);
}

void Flash::Deserialize(std::ifstream& saveState)
{
    for (auto& bank : flash_)
    {
        DeserializeArray(bank);
    }

    DeserializeTrivialType(bank_);
    DeserializeTrivialType(state_);
    DeserializeTrivialType(chipIdMode_);
}
}  // namespace cartridge
