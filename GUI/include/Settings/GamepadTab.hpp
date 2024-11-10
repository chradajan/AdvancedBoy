#pragma once

#include <vector>
#include <GUI/include/Bindings.hpp>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QWidget>
#include <SDL2/SDL.h>

class PersistentData;

namespace gui
{
class DeadzoneSlider : public QWidget
{
    Q_OBJECT

public:
    /// @brief Initialize the deadzone slider widget.
    /// @param deadzone Starting value of slider.
    DeadzoneSlider(int deadzone);

    /// @brief Change the slider position and label for a new deadzone percentage.
    /// @param deadzone New deadzone percentage.
    void SetDeadzone(int deadzone);

signals:
    /// @brief Emit when the deadzone value has been changed.
    /// @param deadzone New deadzone percentage.
    void DeadzoneChangedSignal(int deadzone);

private slots:
    /// @brief Internal slot to handle deadzone slider adjustments.
    /// @param deadzone New deadzone percentage.
    void DeadzoneChangedSlot(int deadzone);

private:
    QLabel* label_;
    QSlider* slider_;
};

class GamepadTab : public QWidget
{
    Q_OBJECT

public:
    GamepadTab() = delete;
    GamepadTab(GamepadTab const&) = delete;
    GamepadTab& operator=(GamepadTab const&) = delete;
    GamepadTab(GamepadTab&&) = delete;
    GamepadTab& operator=(GamepadTab&&) = delete;

    /// @brief Initialize the gamepad tab widget.
    /// @param settings Reference to settings.
    GamepadTab(PersistentData& settings);

    /// @brief Update the dropdown of available gamepads.
    void UpdateGamepadList();

    /// @brief Cancel the rebinding of a button.
    void CancelRebind();

    /// @brief Restore default keypad bindings.
    void RestoreDefaults();

    /// @brief Get the currently selected gamepad.
    /// @return Pointer to current gamepad.
    SDL_GameController* GetGamepad() const { return gamepad_; }

signals:
    /// @brief Emit to tell the gamepad listener to listen for the next axis/button event.
    /// @param id Joystick ID of the gamepad to listen to.
    void GetNewGamepadBindingSignal(SDL_JoystickID id);

    /// @brief Emit to notify the main window which gamepad it should poll for user inputs.
    /// @param gamepad Pointer to current gamepad.
    void SetGamepadSignal(SDL_GameController* gamepad);

    /// @brief Emit to notify the main window that it should get the latest gamepad bindings from persistent data.
    void BindingsChangedSignal();

public slots:
    /// @brief Update a gamepad binding after the gamepad listener signals that a axis/button event occurred.
    /// @param newBinding Gamepad binding to save.
    void SetNewGamepadBindingSlot(GamepadBinding newBinding);

private slots:
    /// @brief Update the current gamepad after the index of the gamepad dropdown changes.
    /// @param index Index of selected gamepad in the dropdown.
    void GamepadSelectionSlot(int index);

    /// @brief Update the saved deadzone and notify the main window that bindings have changed.
    /// @param deadzone New deadzone percentage.
    void DeadzoneChangedSlot(int deadzone);

private:
    /// @brief Create the box with the gamepad dropdown and deadzone slider.
    /// @return Group box to add to main layout.
    [[nodiscard]] QGroupBox* CreateGamepadSelectBox() const;

    /// @brief Create the box with buttons of gamepad to GBA key bindings.
    /// @return Group box to add to main layout.
    [[nodiscard]] QGroupBox* CreateBindingsBox();

    /// @brief Prepare for a gamepad binding to be updated after a button in the bindings box has been clicked.
    /// @param button Pointer to button that was clicked.
    /// @param gbaKey GBA key to set the new binding for.
    /// @param primary Whether it was the primary or secondary binding button that was clicked.
    void PrepareKeyBindingChange(QPushButton* button, GBAKey gbaKey, bool primary);

    /// @brief Update the text on all binding buttons based on bindings and gamepad type.
    void UpdateButtons();

    /// @brief Populate the gamepads dropdown with all connected gamepads.
    void PopulateGamepadDropdown();

    // Current gamepad
    std::vector<int> joystickIndexes_;
    SDL_GameController* gamepad_;
    SDL_JoystickID joystickID_;

    // Rebind info
    QPushButton* buttonToUpdate_;
    GBAKey gbaKeyToUpdate_;
    bool updatePrimary_;
    QString textToRestore_;

    // Settings
    PersistentData& settings_;
};
}
