#pragma once

#include <filesystem>
#include <GBA/include/Utilities/Types.hpp>
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

    /// @brief Run the emulator either indefinitely or for a single frame.
    /// @param stepType Duration to run the emulator.
    void StartEmulator(StepType stepType);

private:
    /// @brief Run emulator in a separate thread.
    void run() override;

    // Data
    StepType stepType_;
};
}  // namespace gui
