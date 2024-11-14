#include <GUI/include/GBA.hpp>
#include <array>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <unordered_set>
#include <GBA/include/Debug/DebugTypes.hpp>
#include <GBA/include/Debug/GameBoyAdvanceDebugger.hpp>
#include <GBA/include/GameBoyAdvance.hpp>
#include <GBA/include/Keypad/Registers.hpp>
#include <GBA/include/Memory/MemoryMap.hpp>
#include <GBA/include/Utilities/Types.hpp>

static std::unique_ptr<GameBoyAdvance> GBA;
static std::unique_ptr<debug::GameBoyAdvanceDebugger> GBADebugger;
static std::unordered_set<u32> EMPTY_SET = {};
static debug::Mnemonic EmptyMnemonic = {"???", "", "???", {}};
static u32 ClockSpeed = 16'777'216;

namespace gba_api
{
void InitializeGBA(fs::path biosPath,
                   fs::path romPath,
                   fs::path saveDir,
                   std::function<void()> vBlankCallback,
                   std::function<void()> breakpointCallback)
{
    if (GBA)
    {
        return;
    }

    GBA = std::make_unique<GameBoyAdvance>(biosPath, romPath, saveDir, vBlankCallback, breakpointCallback);
    GBA->SetCpuClockSpeed(ClockSpeed);
    GBADebugger = std::make_unique<debug::GameBoyAdvanceDebugger>(*GBA);
}

void RunEmulationLoop()
{
    if (!GBA)
    {
        return;
    }

    GBA->Run();
}

void PowerOff()
{
    if (!GBA)
    {
        return;
    }

    GBA.reset();
    GBADebugger.reset();
}

void FillAudioBuffer(u8* stream, size_t len)
{
    float* buffer = reinterpret_cast<float*>(stream);
    size_t cnt = len / sizeof(float);

    if (!GBA)
    {
        std::memset(buffer, 0, cnt * sizeof(float));
        return;
    }

    size_t availableSamples = GBA->AvailableSamples();

    if (cnt > availableSamples)
    {
        GBA->DrainAudioBuffer(buffer, availableSamples);
        std::memset(&buffer[availableSamples], 0, (cnt - availableSamples) * sizeof(float));
    }
    else
    {
        GBA->DrainAudioBuffer(buffer, cnt);
    }
}

uchar* GetFrameBuffer()
{
    static std::array<u16, 240 * 160> BLANK_SCREEN = {};

    if (!GBA)
    {
        return reinterpret_cast<uchar*>(BLANK_SCREEN.data());
    }

    return GBA->GetRawFrameBuffer();
}

int GetFPSCounter()
{
    if (!GBA)
    {
        return 0;
    }

    return GBA->GetFPSCounter();
}

std::string GetTitle()
{
    if (!GBA)
    {
        return "Advanced Boy";
    }

    std::string title = GBA->GetTitle();

    if (title == "")
    {
        return "Advanced Boy";
    }

    return title;
}

void UpdateKeypad(KEYINPUT keyinput)
{
    if (GBA)
    {
        GBA->UpdateKeypad(keyinput);
    }
}

void SetCpuClockSpeed(u32 clockSpeed)
{
    if (GBA)
    {
        GBA->SetCpuClockSpeed(clockSpeed);
    }

    ClockSpeed = clockSpeed;
}

///---------------------------------------------------------------------------------------------------------------------------------
/// Audio
///---------------------------------------------------------------------------------------------------------------------------------

void SetVolume(bool mute, int volume)
{
    if (GBA)
    {
        GBA->SetVolume(mute, volume);
    }
}

void SetAPUChannels(bool channel1, bool channel2, bool channel3, bool channel4, bool fifoA, bool fifoB)
{
    if (GBA)
    {
        GBA->SetAPUChannels(channel1, channel2, channel3, channel4, fifoA, fifoB);
    }
}

///---------------------------------------------------------------------------------------------------------------------------------
/// Validity checks
///---------------------------------------------------------------------------------------------------------------------------------

bool ValidBiosLoaded()
{
    return GBA ? GBA->ValidBiosLoaded() : false;
}

bool ValidGamePakLoaded()
{
    return GBA ? GBA->ValidGamePakLoaded() : false;
}

///---------------------------------------------------------------------------------------------------------------------------------
/// Save states
///---------------------------------------------------------------------------------------------------------------------------------

fs::path GetSavePath()
{
    return GBA ? GBA->GetSavePath() : "";
}

void CreateSaveState(std::ofstream& saveState)
{
    if (GBA)
    {
        GBA->Serialize(saveState);
    }
}

void LoadSaveState(std::ifstream& saveState)
{
    if (GBA)
    {
        GBA->Deserialize(saveState);
    }
}

///---------------------------------------------------------------------------------------------------------------------------------
/// Debug
///---------------------------------------------------------------------------------------------------------------------------------

void StepCPU()
{
    if (GBA)
    {
        GBA->StepCPU();
    }
}

void StepFrame()
{
    if (GBA)
    {
        GBA->StepFrame();
    }
}

void GetBgDebugInfo(debug::BackgroundDebugInfo& debugInfo, u8 bgIndex)
{
    if (GBADebugger)
    {
        GBADebugger->GetBgDebugInfo(debugInfo, bgIndex);
    }
    else
    {
        debugInfo.width = 0;
        debugInfo.height = 0;
    }
}

void GetSpriteDebugInfo(debug::SpriteDebugInfo& sprites, bool regTransforms, bool affTransforms)
{
    if (GBADebugger)
    {
        GBADebugger->GetSpriteDebugInfo(sprites, regTransforms, affTransforms);
    }
    else
    {
        for (auto& sprite : sprites)
        {
            sprite.enabled = false;
        }
    }
}

debug::CpuDebugInfo GetCpuDebugInfo()
{
    if (GBADebugger)
    {
        return GBADebugger->GetCpuDebugInfo();
    }

    debug::CpuDebugInfo emptyDebugInfo = {};
    emptyDebugInfo.pcMem.page = Page::INVALID;
    emptyDebugInfo.spMem.page = Page::INVALID;
    return emptyDebugInfo;
}

debug::Mnemonic const& DisassembleArmInstruction(u32 instruction)
{
    if (GBADebugger)
    {
        return GBADebugger->DisassembleArmInstruction(instruction);
    }

    return EmptyMnemonic;
}

debug::Mnemonic const& DisassembleThumbInstruction(u32 instruction)
{
    if (GBADebugger)
    {
        return GBADebugger->DisassembleThumbInstruction(instruction);
    }

    return EmptyMnemonic;
}

void SetBreakpoint(u32 breakpoint)
{
    if (GBA)
    {
        GBA->SetBreakpoint(breakpoint);
    }
}

void RemoveBreakpoint(u32 breakpoint)
{
    if (GBA)
    {
        GBA->RemoveBreakpoint(breakpoint);
    }
}

std::unordered_set<u32> const& GetBreakpoints()
{
    if (GBA)
    {
        return GBA->GetBreakpoints();
    }

    return EMPTY_SET;
}

u32 DebugReadRegister(u32 addr, u8 size)
{
    if (GBADebugger)
    {
        return GBADebugger->ReadRegister(addr, AccessSize(size));
    }

    return 0;
}
}  // namespace gui
