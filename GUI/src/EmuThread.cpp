#include <GUI/include/EmuThread.hpp>
#include <GBA/include/Utilities/Types.hpp>
#include <GUI/include/GBA.hpp>
#include <QtCore/QThread>

namespace gui
{
void EmuThread::StartEmulator(StepType stepType)
{
    stepType_ = stepType;
    start();
}

void EmuThread::run()
{
    switch (stepType_)
    {
        case StepType::Run:
        {
            while (!isInterruptionRequested())
            {
                gba_api::RunEmulationLoop();
                msleep(5);
            }

            break;
        }
        case StepType::CpuStep:
            ::gba_api::StepCPU();
            break;
        case StepType::FrameStep:
            ::gba_api::StepFrame();
            break;
    }
}
}  // namespace gui
