#pragma once

#include <filesystem>
#include <memory>
#include <set>
#include <string>
#include <GBA/include/Utilities/Types.hpp>
#include <GUI/include/DebugWindows/BackgroundViewerWindow.hpp>
#include <GUI/include/DebugWindows/CpuDebuggerWindow.hpp>
#include <GUI/include/DebugWindows/RegisterViewerWindow.hpp>
#include <GUI/include/DebugWindows/SpriteViewerWindow.hpp>
#include <GUI/include/EmuThread.hpp>
#include <GUI/include/LCD.hpp>
#include <QtCore/QTimer>
#include <QtWidgets/QMainWindow>
#include <SDL2/SDL.h>

namespace fs = std::filesystem;

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

    /// @brief Stop the currently running GBA if one exists, create a new GBA, and start the main emulation loop.
    /// @param romPath Path to GBA ROM.
    void StartEmulation(fs::path romPath);

    /// @brief Update the path to the BIOS file.
    /// @param biosPath Path to GBA BIOS.
    void SetBiosPath(fs::path biosPath) { biosPath_ = biosPath; }

public slots:
    /// @brief Slot to handle emulator control by the CPU Debugger Window.
    /// @param stepType Duration to run the emulator for.
    void CpuDebugStepSlot(StepType stepType);

signals:
    /// @brief Emit this signal to notify the Background Viewer to update its displayed image/data.
    /// @param updateBg Whether to fetch the latest background data from the GBA.
    void UpdateBackgroundViewSignal(bool updateBg);

    /// @brief Emit this signal to notify the Sprite Viewer to update its displayed image/data.
    /// @param updateSprites Whether to update the content of sprites_ with the latest OAM data.
    void UpdateSpriteViewerSignal(bool updateSprites);

    /// @brief Emit this signal to notify the CPU Debugger to update its displayed data.
    void UpdateCpuDebuggerSignal();

    /// @brief Emit this signal to notify the Register Viewer to update its displayed data.
    void UpdateRegisterViewerSignal();

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
    void VBlankCallback();

    /// @brief Callback function for whenever a breakpoint set in the CPU debugger is encountered.
    void BreakpointCallback();

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Emulation management
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Update the GBA keypad based on which keys are currently pressed.
    void SendKeyPresses();

    /// @brief If the emulator thread isn't already running, start it and the audio callback thread.
    void StartEmulationThreads();

    /// @brief If the emulator thread is running, stop it and the audio callback thread.
    void StopEmulationThreads();

    /// @brief Pause emulator if it's running and update emulation menu pause item.
    void PauseEmulation();

    /// @brief Resume emulator if it's current paused and update emulation menu pause item.
    void ResumeEmulation();

    /// @brief Speed up or slow down emulation by setting the CPU clock speed.
    /// @param cpuClockSpeed New CPU clock speed in Hz.
    void SetEmulationSpeed(u32 cpuClockSpeed);

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Menu bars
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Initialize the menu bar options.
    void InitializeMenuBar();

    /// @brief Create items under the File menu option.
    /// @return Pointer to file menu.
    [[nodiscard]] QMenu* CreateFileMenu();

    /// @brief Create items under the Emulation menu option.
    /// @return Pointer to emulation menu.
    [[nodiscard]] QMenu* CreateEmulationMenu();

    /// @brief Create items under the Debug menu option.
    /// @return Pointer to debug menu.
    [[nodiscard]] QMenu* CreateDebugMenu();

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Actions
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Action for clicking "Pause" menu item.
    void PauseButtonAction();

    /// @brief Action for clicking "View BG Maps" menu item.
    void OpenBgMapsWindow();

    /// @brief Action for clicking "View Sprites" menu item.
    void OpenSpriteViewerWindow();

    /// @brief Action for clicking "CPU Debugger" menu item.
    void OpenCpuDebuggerWindow();

    /// @brief Action for clicking "I/O Registers" menu item.
    void OpenRegisterViewerWindow();

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Data
    ///-----------------------------------------------------------------------------------------------------------------------------

    // Emulation control
    EmuThread emuThread_;
    fs::path biosPath_;
    bool stepFrameMode_;

    // Screen control
    LCD screen_;

    // Audio control
    SDL_AudioDeviceID audioDevice_;

    // Window title
    QTimer fpsTimer_;
    std::string romTitle_;

    // Keypad
    std::set<int> pressedKeys_;

    // Emulation menu
    QAction* pauseButton_;

    // Debug menus
    std::unique_ptr<BackgroundViewerWindow> bgViewerWindow_;
    std::unique_ptr<SpriteViewerWindow> spriteViewerWindow_;
    std::unique_ptr<CpuDebuggerWindow> cpuDebuggerWindow_;
    std::unique_ptr<RegisterViewerWindow> registerViewerWindow_;
};
}  // namespace gui
