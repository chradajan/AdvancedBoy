#include <GBA/include/APU/APU.hpp>
#include <array>
#include <cstddef>
#include <functional>
#include <GBA/include/System/EventScheduler.hpp>
#include <GBA/include/Types.hpp>

namespace audio
{
APU::APU(EventScheduler& scheduler) : scheduler_(scheduler)
{
    registers_.fill(std::byte{0});

    scheduler_.RegisterEvent(EventType::SampleAPU, std::bind(&APU::Sample, this, std::placeholders::_1));
    scheduler.ScheduleEvent(EventType::SampleAPU, CPU_CYCLES_PER_SAMPLE);
}

MemReadData APU::ReadReg(u32 addr, AccessSize length)
{
    (void)addr;
    (void)length;
    return {1, 0, false};
}

int APU::WriteReg(u32 addr, u32 val, AccessSize length)
{
    (void)addr;
    (void)val;
    (void)length;
    return 1;
}

void APU::Sample(int extraCycles)
{
    float sample[2] = {0.0, 0.0};
    sampleBuffer_.Write(sample, 2);

    ++sampleCounter_;
    scheduler_.ScheduleEvent(EventType::SampleAPU, CPU_CYCLES_PER_SAMPLE - extraCycles);
}
}  // namespace audio
