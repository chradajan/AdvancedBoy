#pragma once

#include <filesystem>
#include <set>
#include <string>
#include <GUI/include/EmuThread.hpp>
#include <GUI/include/LCD.hpp>
#include <SDL2/SDL.h>
#include <QtCore/QTimer>
#include <QtWidgets/QMainWindow>

namespace fs = std::filesystem;
namespace gui { class BackgroundViewer; }
namespace gui { class CpuDebugger; }

namespace gui
{
/// @brief Main window of the GUI.
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /// @brief Initialize GUI.
    /// @param parent Parent widget.
    MainWindow(QWidget* parent = nullptr);

    /// @brief Main window destructor.
    ~MainWindow();

    /// @brief Stop the currently running GBA if one exists, create a new GBA, and start the main emulation loop.
    /// @param romPath Path to GBA ROM.
    void StartEmulation(fs::path romPath);

    /// @brief Update the path to the BIOS file.
    /// @param biosPath Path to GBA BIOS.
    void SetBiosPath(fs::path biosPath) { biosPath_ = biosPath; }

public slots:
    /// @brief Pause the emulator if it's running.
    void PauseSlot() { PauseEmulation(); }

    /// @brief Resume the emulator if it's paused.
    void ResumeSlot() { ResumeEmulation(); }

signals:
    /// @brief Emit this signal to notify the Background Viewer to update its displayed image/data.
    void UpdateBackgroundViewSignal();

    /// @brief Emit this signal to notify the CPU Debugger to update its displayed data.
    void UpdateCpuDebuggerSignal();

private:
    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Event handlers
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Event handling when file is dragged into window.
    /// @param event Drag event.
    void dragEnterEvent(QDragEnterEvent* event) override;

    /// @brief Load any dropped GBA ROMs.
    /// @param event Drop event.
    void dropEvent(QDropEvent* event) override;

    /// @brief Add whichever key was pressed to the set of currently pressed keys.
    /// @param event Key press event.
    void keyPressEvent(QKeyEvent* event) override;

    /// @brief Remove whichever key was released from the set of currently pressed keys.
    /// @param event Key release event.
    void keyReleaseEvent(QKeyEvent* event) override;

    /// @brief Stop emulation and shut down GBA when window is closed.
    /// @param event Close event.
    void closeEvent(QCloseEvent* event) override;

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Window management
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Signal the LCD to update the screen.
    void RefreshScreen();

    /// @brief Update the window title with the ROM name and current internal FPS.
    void UpdateWindowTitle();

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Callbacks
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Callback function for whenever the GBA enters VBlank.
    void VBlankCallback(int);

    /// @brief Callback function for whenever a breakpoint set in the CPU debugger is encountered.
    void BreakpointCallback();

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Emulation management
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Update the GBA keypad based on which keys are currently pressed.
    void SendKeyPresses();

    /// @brief Pause emulator if it's running and update emulation menu pause item.
    void PauseEmulation();

    /// @brief 
    void ResumeEmulation();

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Menu bars
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Initialize the menu bar options.
    void InitializeMenuBar();

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Actions
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Action for clicking "Pause" menu item.
    void PauseButtonAction();

    /// @brief Action for clicking "View BG Maps" menu item.
    void OpenBgMapsWindow();

    /// @brief Action for clicking "CPU Debugger" menu item.
    void OpenCpuDebugger();

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Data
    ///-----------------------------------------------------------------------------------------------------------------------------

    // Emulation control
    EmuThread emuThread_;
    fs::path biosPath_;
    fs::path logDir_;

    // Screen control
    LCD screen_;

    // Audio control
    SDL_AudioDeviceID audioDevice_;

    // Window title
    QTimer fpsTimer_;
    std::string romTitle_;

    // Keypad
    std::set<int> pressedKeys_;

    // Menu bar
    QMenu* fileMenu_;
    QMenu* emulationMenu_;
    QMenu* debugMenu_;
    QMenu* optionsMenu_;

    // Emulation menu
    QAction* pauseAction_;

    // Debug menus
    BackgroundViewer* bgMapsWindow_;
    CpuDebugger* cpuDebugWindow_;
};
}  // namespace gui
