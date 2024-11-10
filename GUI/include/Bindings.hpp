#pragma once

#include <array>
#include <utility>
#include <GBA/include/Utilities/Types.hpp>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <SDL2/SDL.h>

namespace gui
{
enum class GBAKey
{
    UP, DOWN, LEFT, RIGHT, L, R, A, B, START, SELECT, INVALID
};

/// @brief Get the human readable name of a keyboard binding.
/// @param key Key enum value of keyboard key.
/// @return Name of key.
QString GetKeyboardBindingName(Qt::Key key);

class GamepadBinding
{
public:
    /// @brief Construct an invalid binding that never reports being active.
    GamepadBinding() : valid_(false) {}

    /// @brief Construct a gamepad binding for a button.
    /// @param button Gamepad button.
    explicit GamepadBinding(SDL_GameControllerButton button) : valid_(true), isButton_(true), value_(button) {}

    /// @brief Construct a gamepad binding for an axis.
    /// @param axis Gamepad axis.
    /// @param positive Axis direction.
    /// @param deadzone Current deadzone value [0-100].
    explicit GamepadBinding(SDL_GameControllerAxis axis, bool positive, int deadzone = 0);

    /// @brief Construct a gamepad binding from a list of variants from persistent data.
    /// @param list List of QVariants in the following format:
    ///  Button binding: [true (bool), SDL_GameControllerButton (int)]
    ///  Axis binding:   [false (bool), SDL_GameControllerAxis (int), positive (bool)]
    ///  Null binding:   [QVariant()]
    /// @param deadzone Current deadzone value [0-100].
    explicit GamepadBinding(QVariantList const& list, int deadzone);

    /// @brief Check if the button is pressed or if the axis is outside the deadzone.
    /// @param gamepad Pointer to gamepad to check inputs on.
    /// @return Whether this binding is active on the specified gamepad.
    bool Active(SDL_GameController* gamepad) const;

    /// @brief Get a human readable description of the binding.
    /// @param gamepad Optionally pointer to gamepad to customize axis/button names for.
    /// @return String version of binding.
    QString ToString(SDL_GameController* gamepad = nullptr) const;

    /// @brief Convert binding into a list of QVariant for recording on persistent data.
    ///  Button binding: [true (bool), SDL_GameControllerButton (int)]
    ///  Axis binding:   [false (bool), SDL_GameControllerAxis (int), positive (bool)]
    ///  Null binding:   [QVariant()]
    /// @return List of QVariants.
    QVariantList ToList() const;

private:
    /// @brief Set the value of axisThreshold_.
    /// @param axis Which axis to calculate the threshold for. Only apply deadzone for joysticks.
    /// @param positive Axis direction.
    /// @param deadzone Deadzone value [0-100].
    static i16 CalculateAxisThreshold(SDL_GameControllerAxis axis, bool positive, int deadzone);

    bool valid_;
    bool isButton_;
    int value_;
    bool positive_;
    int deadzone_;
    i16 axisThreshold_;
};

struct GamepadMap
{
    std::pair<GamepadBinding, GamepadBinding> up;
    std::pair<GamepadBinding, GamepadBinding> down;
    std::pair<GamepadBinding, GamepadBinding> left;
    std::pair<GamepadBinding, GamepadBinding> right;
    std::pair<GamepadBinding, GamepadBinding> l;
    std::pair<GamepadBinding, GamepadBinding> r;
    std::pair<GamepadBinding, GamepadBinding> a;
    std::pair<GamepadBinding, GamepadBinding> b;
    std::pair<GamepadBinding, GamepadBinding> start;
    std::pair<GamepadBinding, GamepadBinding> select;
};

struct KeyboardMap
{
    std::pair<Qt::Key, Qt::Key> up;
    std::pair<Qt::Key, Qt::Key> down;
    std::pair<Qt::Key, Qt::Key> left;
    std::pair<Qt::Key, Qt::Key> right;
    std::pair<Qt::Key, Qt::Key> l;
    std::pair<Qt::Key, Qt::Key> r;
    std::pair<Qt::Key, Qt::Key> a;
    std::pair<Qt::Key, Qt::Key> b;
    std::pair<Qt::Key, Qt::Key> start;
    std::pair<Qt::Key, Qt::Key> select;
};

extern const std::array<std::pair<QString, GBAKey>, 10> BUTTON_NAMES;
}
