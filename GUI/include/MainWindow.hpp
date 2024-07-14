#pragma once

#include <filesystem>
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

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Event handlers
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Event handling when file is dragged into window.
    /// @param event Drag event.
    void dragEnterEvent(QDragEnterEvent* event);

    /// @brief Load any dropped GBA ROMs.
    /// @param event Drop event.
    void dropEvent(QDropEvent* event);

private:
    /// @brief Stop the currently running GBA if one exists, create a new GBA, and start the main emulation loop.
    /// @param romPath Path to GBA ROM.
    void StartEmulation(fs::path romPath);

    /// @brief Signal the LCD to update the screen.
    void RefreshScreen();

    // Emulation control
    EmuThread emuThread_;

    // Screen control
    LCD screen_;
    QTimer screenTimer_;

    // Audio control
    SDL_AudioDeviceID audioDevice_;
};
}  // namespace gui
