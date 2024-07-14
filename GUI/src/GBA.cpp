#include <GUI/include/GBA.hpp>
#include <filesystem>
#include <array>
#include <cstring>
#include <memory>
#include <GBA/include/GameBoyAdvance.hpp>

std::unique_ptr<GameBoyAdvance> GBA;

namespace gui
{
void InitializeGBA(fs::path biosPath, fs::path romPath, fs::path logDir)
{
    if (GBA)
    {
        return;
    }

    GBA = std::make_unique<GameBoyAdvance>(biosPath, romPath, logDir);
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

u8* GetFrameBuffer()
{
    static std::array<u16, 240 * 160> BLANK_SCREEN = {};

    if (!GBA)
    {
        return reinterpret_cast<u8*>(BLANK_SCREEN.data());
    }

    return GBA->GetRawFrameBuffer();
}
}  // namespace gui
