#include <GUI/include/EmuThread.hpp>
#include <GUI/include/GBA.hpp>
#include <QtCore/QThread>

namespace gui
{
void EmuThread::run()
{
    while (!isInterruptionRequested())
    {
        gba_api::RunEmulationLoop();
        msleep(5);
    }
}
}  // namespace gui
