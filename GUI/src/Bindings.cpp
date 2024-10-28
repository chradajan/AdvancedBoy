#include <GUI/include/Bindings.hpp>
#include <GBA/include/Utilities/Types.hpp>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <SDL2/SDL.h>

namespace
{
/// @brief Check if a gamepad is a Nintendo gamepad.
/// @param controllerName Name of gamepad.
/// @return True if gamepad is a Nintendo gamepad.
bool IsNintendoController(QString controllerName)
{
    return controllerName.contains("Nintendo", Qt::CaseInsensitive);
}

/// @brief Check if a gamepad is a Sony gamepad.
/// @param controllerName Name of gamepad.
/// @return True if gamepad is a Sony gamepad.
bool IsSonyController(QString controllerName)
{
    return controllerName.contains("DualSense", Qt::CaseInsensitive);
}

/// @brief Get a human readable name of a Nintendo button.
/// @param button SDL button enum value.
/// @return Nintendo button name.
QString GetNintendoButtonName(SDL_GameControllerButton button)
{
    switch (button)
    {
        case SDL_CONTROLLER_BUTTON_A:
            return "B";
        case SDL_CONTROLLER_BUTTON_B:
            return "A";
        case SDL_CONTROLLER_BUTTON_X:
            return "Y";
        case SDL_CONTROLLER_BUTTON_Y:
            return "X";
        case SDL_CONTROLLER_BUTTON_BACK:
            return "-";
        case SDL_CONTROLLER_BUTTON_GUIDE:
            return "Home";
        case SDL_CONTROLLER_BUTTON_START:
            return "+";
        case SDL_CONTROLLER_BUTTON_LEFTSTICK:
            return "Left Stick";
        case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
            return "Right Stick";
        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
            return "L";
        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
            return "R";
        case SDL_CONTROLLER_BUTTON_DPAD_UP:
            return "DPad Up";
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
            return "DPad Down";
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
            return "DPad Left";
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
            return "DPad Right";
        case SDL_CONTROLLER_BUTTON_MISC1:
            return "Capture";
        default:
            break;
    }

    return "Unknown Button";
}

/// @brief Get a human readable name of a Sony button.
/// @param button SDL button enum value.
/// @return Sony button name.
QString GetSonyButtonName(SDL_GameControllerButton button)
{
    switch (button)
    {
        case SDL_CONTROLLER_BUTTON_A:
            return "Cross";
        case SDL_CONTROLLER_BUTTON_B:
            return "Circle";
        case SDL_CONTROLLER_BUTTON_X:
            return "Square";
        case SDL_CONTROLLER_BUTTON_Y:
            return "Triangle";
        case SDL_CONTROLLER_BUTTON_BACK:
            return "Create";
        case SDL_CONTROLLER_BUTTON_GUIDE:
            return "Home";
        case SDL_CONTROLLER_BUTTON_START:
            return "Options";
        case SDL_CONTROLLER_BUTTON_LEFTSTICK:
            return "L3";
        case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
            return "R3";
        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
            return "L1";
        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
            return "R1";
        case SDL_CONTROLLER_BUTTON_DPAD_UP:
            return "DPad Up";
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
            return "DPad Down";
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
            return "DPad Left";
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
            return "DPad Right";
        case SDL_CONTROLLER_BUTTON_MISC1:
            return "Mute";
        case SDL_CONTROLLER_BUTTON_TOUCHPAD:
            return "Touchpad";
        default:
            break;
    }

    return "Unknown Button";
}

/// @brief Get a human readable name of an XBox button (also use for unknown gamepad).
/// @param button SDL button enum value.
/// @return Xbox button name.
QString GetXboxButtonName(SDL_GameControllerButton button)
{
    switch (button)
    {
        case SDL_CONTROLLER_BUTTON_A:
            return "A";
        case SDL_CONTROLLER_BUTTON_B:
            return "B";
        case SDL_CONTROLLER_BUTTON_X:
            return "X";
        case SDL_CONTROLLER_BUTTON_Y:
            return "Y";
        case SDL_CONTROLLER_BUTTON_BACK:
            return "View";
        case SDL_CONTROLLER_BUTTON_GUIDE:
            return "Xbox";
        case SDL_CONTROLLER_BUTTON_START:
            return "Menu";
        case SDL_CONTROLLER_BUTTON_LEFTSTICK:
            return "Left Stick";
        case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
            return "Right Stick";
        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
            return "LB";
        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
            return "RB";
        case SDL_CONTROLLER_BUTTON_DPAD_UP:
            return "DPad Up";
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
            return "DPad Down";
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
            return "DPad Left";
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
            return "DPad Right";
        case SDL_CONTROLLER_BUTTON_MISC1:
            return "Share";
        case SDL_CONTROLLER_BUTTON_PADDLE1:
            return "P1";
        case SDL_CONTROLLER_BUTTON_PADDLE2:
            return "P2";
        case SDL_CONTROLLER_BUTTON_PADDLE3:
            return "P3";
        case SDL_CONTROLLER_BUTTON_PADDLE4:
            return "P4";
        case SDL_CONTROLLER_BUTTON_TOUCHPAD:
            return "Touchpad";
        default:
            break;
    }

    return "Unknown Button";
}

/// @brief Get a human readable name of a Nintendo axis.
/// @param axis SDL axis enum value.
/// @return Nintendo axis name.
QString GetNintendoAxisName(SDL_GameControllerAxis axis, bool positive)
{
    QString axisDir = positive ? "+" : "-";

    switch (axis)
        {
            case SDL_CONTROLLER_AXIS_LEFTX:
                return "LX" + axisDir;
            case SDL_CONTROLLER_AXIS_LEFTY:
                return "LY" + axisDir;
            case SDL_CONTROLLER_AXIS_RIGHTX:
                return "RX" + axisDir;
            case SDL_CONTROLLER_AXIS_RIGHTY:
                return "RY" + axisDir;
            case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
                return "ZL";
            case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
                return "ZR";
            default:
                return "Unknown Axis";
        }
}

/// @brief Get a human readable name of a Sony axis.
/// @param axis SDL axis enum value.
/// @return Sony axis name.
QString GetSonyAxisName(SDL_GameControllerAxis axis, bool positive)
{
    QString axisDir = positive ? "+" : "-";

    switch (axis)
        {
            case SDL_CONTROLLER_AXIS_LEFTX:
                return "LX" + axisDir;
            case SDL_CONTROLLER_AXIS_LEFTY:
                return "LY" + axisDir;
            case SDL_CONTROLLER_AXIS_RIGHTX:
                return "RX" + axisDir;
            case SDL_CONTROLLER_AXIS_RIGHTY:
                return "RY" + axisDir;
            case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
                return "L2";
            case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
                return "R2";
            default:
                return "Unknown Axis";
        }
}

/// @brief Get a human readable name of an XBox axis (also use for unknown gamepad).
/// @param axis SDL axis enum value.
/// @return Xbox axis name.
QString GetXboxAxisName(SDL_GameControllerAxis axis, bool positive)
{
    QString axisDir = positive ? "+" : "-";

    switch (axis)
        {
            case SDL_CONTROLLER_AXIS_LEFTX:
                return "LX" + axisDir;
            case SDL_CONTROLLER_AXIS_LEFTY:
                return "LY" + axisDir;
            case SDL_CONTROLLER_AXIS_RIGHTX:
                return "RX" + axisDir;
            case SDL_CONTROLLER_AXIS_RIGHTY:
                return "RY" + axisDir;
            case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
                return "LT";
            case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
                return "RT";
            default:
                return "Unknown Axis";
        }
}
}

namespace gui
{
static_assert(sizeof(Sint16) == sizeof(i16), "Internal i16 size does not match SDL Sint16 size");

GamepadBinding::GamepadBinding(SDL_GameControllerAxis axis, bool positive, int deadzone) :
    valid_(true),
    isButton_(false),
    value_(axis),
    positive_(positive),
    deadzone_(deadzone)
{
    if ((deadzone < 0) || (deadzone > 100))
    {
        valid_ = false;
        return;
    }

    axisThreshold_ = CalculateAxisThreshold(axis, positive, deadzone);
}

GamepadBinding::GamepadBinding(QVariantList const& list, int deadzone)
{
    if ((list.length() == 0) || !list[0].isValid())
    {
        valid_ = false;
        return;
    }

    valid_ = true;
    isButton_ = list[0].toBool();
    value_ = list[1].toInt();

    if (!isButton_)
    {
        positive_ = list[2].toBool();
        deadzone_ = deadzone;
        axisThreshold_ = CalculateAxisThreshold(static_cast<SDL_GameControllerAxis>(value_), positive_, deadzone_);
    }
}

bool GamepadBinding::Active(SDL_GameController* gamepad) const
{
    if (!valid_)
    {
        return false;
    }

    if (isButton_)
    {
        return SDL_GameControllerGetButton(gamepad, static_cast<SDL_GameControllerButton>(value_));
    }

    Sint16 axisVal = SDL_GameControllerGetAxis(gamepad, static_cast<SDL_GameControllerAxis>(value_));
    return positive_ ? (axisVal >= axisThreshold_) : (axisVal <= axisThreshold_);
}

QString GamepadBinding::ToString(SDL_GameController* gamepad) const
{
    if (!valid_ || (gamepad == nullptr))
    {
        return "";
    }

    QString controllerName = SDL_GameControllerName(gamepad);

    if (isButton_)
    {
        auto button = static_cast<SDL_GameControllerButton>(value_);

        if (IsNintendoController(controllerName))
        {
            return GetNintendoButtonName(button);
        }
        else if (IsSonyController(controllerName))
        {
            return GetSonyButtonName(button);
        }

        return GetXboxButtonName(button);
    }
    else
    {
        auto axis = static_cast<SDL_GameControllerAxis>(value_);

        if (IsNintendoController(controllerName))
        {
            return GetNintendoAxisName(axis, positive_);
        }
        else if (IsSonyController(controllerName))
        {
            return GetSonyAxisName(axis, positive_);
        }

        return GetXboxAxisName(axis, positive_);
    }

    return "Unknown";
}

QVariantList GamepadBinding::ToList() const
{
    if (!valid_)
    {
        return QVariantList{QVariant()};
    }

    if (isButton_)
    {
        return QVariantList{true, value_};
    }

    return QVariantList{false, value_, positive_};
}

i16 GamepadBinding::CalculateAxisThreshold(SDL_GameControllerAxis axis, bool positive, int deadzone)
{
    if ((axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT) || (axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT))
    {
        // Always use 5% deadzone for triggers.
        return I16_MAX * 0.05f;
    }

    if (deadzone == 0)
    {
        return positive ? 1 : -1;
    }
    else if (deadzone == 100)
    {
        return positive ? I16_MAX : I16_MIN;
    }

    float deadzoneFraction = static_cast<float>(deadzone) / 100.0f;
    return positive ? (I16_MAX * deadzoneFraction) : (I16_MIN * deadzoneFraction);
}
}
