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
    /// Emulation management
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Update the GBA keypad based on which keys are currently pressed.
    void SendKeyPresses();

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Data
    ///-----------------------------------------------------------------------------------------------------------------------------

    // Emulation control
    EmuThread emuThread_;
    fs::path biosPath_;
    fs::path logDir_;

    // Screen control
    LCD screen_;
    QTimer screenTimer_;

    // Audio control
    SDL_AudioDeviceID audioDevice_;

    // Window title
    QTimer fpsTimer_;
    std::string romTitle_;

    // Keypad
    std::set<int> pressedKeys_;
};
}  // namespace gui
