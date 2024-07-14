#include <GUI/include/EmuThread.hpp>
#include <GUI/include/GBA.hpp>
#include <QtCore/QThread>

namespace gui
{
void EmuThread::run()
{
    while (!isInterruptionRequested())
    {
        RunEmulationLoop();
        msleep(5);
    }
}
}  // namespace gui
