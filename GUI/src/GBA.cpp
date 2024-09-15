#include <GUI/include/GBA.hpp>
#include <filesystem>
#include <array>
#include <cstring>
#include <memory>
#include <string>
#include <unordered_set>
#include <GBA/include/Keypad/Registers.hpp>
#include <GBA/include/Memory/MemoryMap.hpp>
#include <GBA/include/GameBoyAdvance.hpp>
#include <GBA/include/Types/DebugTypes.hpp>
#include <GBA/include/Types/Types.hpp>

static std::unique_ptr<GameBoyAdvance> GBA;
static std::unordered_set<u32> EMPTY_SET = {};
static debug::cpu::Mnemonic EmptyMnemonic = {"???", "", "???", {}};

namespace gba_api
{
void InitializeGBA(fs::path biosPath,
                   fs::path romPath,
                   fs::path logDir,
                   std::function<void(int)> vBlankCallback,
                   std::function<void()> breakpointCallback)
{
    if (GBA)
    {
        return;
    }

    GBA = std::make_unique<GameBoyAdvance>(biosPath, romPath, logDir, vBlankCallback, breakpointCallback);
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
}

void FillAudioBuffer(u8* stream, size_t len)
{
    if (!GBA)
    {
        return;
    }

    float* buffer = reinterpret_cast<float*>(stream);
    size_t cnt = len / sizeof(float);
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

void RunDisassembler()
{
    if (GBA)
    {
        GBA->RunDisassembler();
    }
}

void SingleStep()
{
    if (GBA)
    {
        GBA->SingleStep();
    }
}

debug::graphics::BackgroundDebugInfo GetBgDebugInfo(u8 bgIndex)
{
    if (GBA)
    {
        return GBA->GetBgDebugInfo(bgIndex);
    }

    return {};
}

debug::cpu::CpuDebugInfo GetCpuDebugInfo()
{
    if (GBA)
    {
        return GBA->GetCpuDebugInfo();
    }

    debug::cpu::CpuDebugInfo emptyDebugInfo = {};
    emptyDebugInfo.pcMem.page = Page::INVALID;
    emptyDebugInfo.spMem.page = Page::INVALID;
    return emptyDebugInfo;
}

debug::cpu::Mnemonic const& DisassembleArmInstruction(u32 instruction)
{
    if (GBA)
    {
        return GBA->DisassembleArmInstruction(instruction);
    }

    return EmptyMnemonic;
}

debug::cpu::Mnemonic const& DisassembleThumbInstruction(u32 instruction)
{
    if (GBA)
    {
        return GBA->DisassembleThumbInstruction(instruction);
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
}  // namespace gui
