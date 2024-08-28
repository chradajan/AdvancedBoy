#include <GUI/include/GBA.hpp>
#include <filesystem>
#include <array>
#include <cstring>
#include <memory>
#include <string>
#include <GBA/include/PPU/Debug.hpp>
#include <GBA/include/GameBoyAdvance.hpp>

std::unique_ptr<GameBoyAdvance> GBA;

namespace gui
{
void InitializeGBA(fs::path biosPath, fs::path romPath, fs::path logDir, std::function<void(int)> vBlankCallback)
{
    if (GBA)
    {
        return;
    }

    GBA = std::make_unique<GameBoyAdvance>(biosPath, romPath, logDir, vBlankCallback);
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

graphics::BackgroundDebugInfo GetBgDebugInfo(u8 bgIndex)
{
    if (GBA)
    {
        return GBA->GetBgDebugInfo(bgIndex);
    }

    return {};
}
}  // namespace gui
