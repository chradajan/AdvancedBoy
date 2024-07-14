#pragma once

#include <filesystem>
#include <QtCore/QThread>

class GameBoyAdvance;
namespace fs = std::filesystem;

namespace gui
{
/// @brief Thread class to keep the main emulation loop running on its own thread.
class EmuThread : public QThread
{
    Q_OBJECT

public:
    /// @brief Initialize EmuThread.
    /// @param parent Pointer to parent object.
    EmuThread(QObject* parent = nullptr) : QThread(parent) {}

private:
    /// @brief Run the emulation loop indefinitely until an interrupt is requested.
    void run() override;
};
}  // namespace gui
