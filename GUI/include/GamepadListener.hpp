#pragma once

#include <GUI/include/Bindings.hpp>
#include <QtCore/QThread>
#include <SDL2/SDL.h>

namespace gui
{
class GamepadListener : public QThread
{
    Q_OBJECT

public:
    /// @brief 
    /// @param parent 
    GamepadListener(QObject* parent = nullptr) : QThread(parent), rebindGamepadId_(-1) {}

signals:
    /// @brief Emit when gamepad is connected.
    /// @param deviceIndex Device index of connected gamepad.
    void GamepadConnectedSignal(int deviceIndex);

    /// @brief Emit when gamepad is disconnected.
    /// @param instanceId Instance ID of disconnected gamepad.
    void GamepadDisconnectedSignal(int instanceId);

    /// @brief Emit when a new gamepad binding has been selected.
    /// @param newBinding Info about the axis/button event.
    void SetNewGamepadBindingSignal(GamepadBinding newBinding);

public slots:
    /// @brief Prepare the listener to handle axis/button events to set a new gamepad binding.
    /// @param id Joystick id of the gamepad to listen to for its next axis/button event.
    void GetNewKeyBindingSlot(SDL_JoystickID id);

private:
    /// @brief Run indefinitely, looking for any gamepads being connected/disconnected.
    void run() override;

    // Data
    SDL_JoystickID rebindGamepadId_;
};
}
