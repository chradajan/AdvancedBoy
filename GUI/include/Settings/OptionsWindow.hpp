#pragma once

#include <GUI/include/Bindings.hpp>
#include <GUI/include/PersistentData.hpp>
#include <QtGui/QCloseEvent>
#include <QtWidgets/QWidget>
#include <SDL2/SDL.h>

namespace gui
{
class OptionsWindow : public QWidget
{
    Q_OBJECT

public:
    OptionsWindow() = delete;
    OptionsWindow(OptionsWindow const&) = delete;
    OptionsWindow& operator=(OptionsWindow const&) = delete;
    OptionsWindow(OptionsWindow&&) = delete;
    OptionsWindow& operator=(OptionsWindow&&) = delete;

    /// @brief Initialize the settings menu.
    OptionsWindow(PersistentData& settings);

    /// @brief Get the currently selected gamepad.
    /// @return Pointer to current gamepad to use for user input.
    SDL_GameController* GetGamepad() const;

signals:
    /// @brief Signal to emit when emulation audio output needs to be adjusted.
    void UpdateAudioSignal(PersistentData::AudioSettings audioSettings);

    /// @brief Emit to signal to the gamepad listener thread to get the next axis/button event.
    /// @param id Joystick ID of gamepad to listen to.
    void GetNewGamepadBindingSignal(SDL_JoystickID id);

    /// @brief Emit to signal the gamepad tab to update a binding.
    /// @param newBinding New binding to save.
    void SetNewGamepadBindingSignal(GamepadBinding newBinding);

    /// @brief Emit to notify the main window which gamepad it should poll for user inputs.
    /// @param gamepad Pointer to current gamepad.
    void SetGamepadSignal(SDL_GameController* gamepad);

    /// @brief Emit to notify the main window that it should get the latest gamepad bindings from persistent data.
    void BindingsChangedSignal();

    /// @brief Emit to notify the keyboard tab to update a binding.
    /// @param key Most recently pressed keyboard key.
    void SetNewKeyboardBindingSignal(Qt::Key key);

    /// @brief Emit when any time related setting has changed.
    void TimeFormatChangedSignal();

public slots:
    /// @brief Slot to handle gamepads being connected/disconnected.
    void UpdateGamepadTabSlot();

private slots:
    /// @brief Determine which tab is currently active and restore its default settings.
    void RestoreDefaultsSlot();

    /// @brief Slot to handle the tab being changed.
    void TabChangedSlot();

    /// @brief Slot to handle changing a keyboard binding.
    void GetNewKeyboardBindingSlot() { listenForKeyPress_ = true; }

private:
    /// @brief Event handler for when the options window closes.
    /// @param event Close event.
    void closeEvent(QCloseEvent* event) override;

    /// @brief When rebinding a key, listen for the next key press.
    /// @param event Key press event.
    void keyPressEvent(QKeyEvent* event) override;

    PersistentData& settings_;
    bool listenForKeyPress_;
};
}
