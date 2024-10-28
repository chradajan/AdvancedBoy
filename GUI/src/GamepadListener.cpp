#include <GUI/include/GamepadListener.hpp>
#include <GBA/include/Utilities/Types.hpp>
#include <GUI/include/Bindings.hpp>
#include <SDL2/SDL.h>

namespace gui
{
void GamepadListener::GetNewKeyBindingSlot(SDL_JoystickID id)
{
    if (rebindGamepadId_ == -1)
    {
        rebindGamepadId_ = id;
    }
}

void GamepadListener::run()
{
    while (!isInterruptionRequested())
    {
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_CONTROLLERDEVICEADDED:
                    emit GamepadConnectedSignal(event.cdevice.which);
                    break;
                case SDL_CONTROLLERDEVICEREMOVED:
                    emit GamepadDisconnectedSignal(event.cdevice.which);
                    break;
                case SDL_CONTROLLERBUTTONDOWN:
                {
                    if (event.cbutton.which == rebindGamepadId_)
                    {
                        rebindGamepadId_ = -1;
                        auto button = static_cast<SDL_GameControllerButton>(event.cbutton.button);
                        emit SetNewGamepadBindingSignal(GamepadBinding(button));
                    }

                    break;
                }
                case SDL_CONTROLLERAXISMOTION:
                {
                    if (event.caxis.which == rebindGamepadId_)
                    {
                        auto axis = static_cast<SDL_GameControllerAxis>(event.caxis.axis);

                        // Use 50% deadzone for determining axis rebind event
                        if (event.caxis.value > (I16_MAX / 2))
                        {
                            rebindGamepadId_ = -1;
                            emit SetNewGamepadBindingSignal(GamepadBinding(axis, true));
                        }
                        else if (event.caxis.value < (I16_MIN / 2))
                        {
                            rebindGamepadId_ = -1;
                            emit SetNewGamepadBindingSignal(GamepadBinding(axis, false));
                        }
                    }

                    break;
                }
            }
        }

        msleep(10);
    }
}
}
