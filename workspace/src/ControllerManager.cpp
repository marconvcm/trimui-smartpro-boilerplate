#include "ControllerManager.hpp"
#include "Logger.hpp"
#include <algorithm>

namespace TG5040
{

    ControllerManager &ControllerManager::getInstance()
    {
        static ControllerManager instance;
        return instance;
    }

    bool ControllerManager::initialize()
    {
        if (initialized_)
        {
            LOG_WARN("ControllerManager already initialized");
            return true;
        }

        LOG_INFO("Initializing ControllerManager");

        // SDL should already be initialized by SDLManager
        // Just scan for existing controllers
        int numJoysticks = SDL_NumJoysticks();
        LOG_INFO("Found %d joystick(s)", numJoysticks);

        for (int i = 0; i < numJoysticks; ++i)
        {
            LOG_DEBUG("Checking joystick %d: IsGameController=%s", i, SDL_IsGameController(i) ? "true" : "false");
            if (SDL_IsGameController(i))
            {
                addController(i);
            }
            else
            {
                // Log joystick info for debugging
                const char* name = SDL_JoystickNameForIndex(i);
                LOG_WARN("Joystick %d ('%s') is not recognized as a game controller, adding as basic joystick", i, name ? name : "Unknown");
                addJoystick(i);
            }
        }

        initialized_ = true;
        LOG_INFO("ControllerManager initialized with %d controller(s)", getControllerCount());
        return true;
    }

    void ControllerManager::shutdown()
    {
        if (!initialized_)
        {
            return;
        }

        LOG_INFO("Shutting down ControllerManager");

        // Close all controllers and joysticks
        for (auto &controller : controllers_)
        {
            if (controller->controller)
            {
                SDL_GameControllerClose(controller->controller);
            }
            else if (controller->joystick)
            {
                SDL_JoystickClose(controller->joystick);
            }
        }

        controllers_.clear();
        buttonStates_.clear();
        axisValues_.clear();
        buttonPressedCallbacks_.clear();
        buttonReleasedCallbacks_.clear();
        buttonEventCallbacks_.clear();
        axisEventCallbacks_.clear();

        initialized_ = false;
    }

    bool ControllerManager::handleEvent(const SDL_Event &event)
    {
        if (!initialized_)
        {
            return false;
        }

        switch (event.type)
        {
        case SDL_CONTROLLERDEVICEADDED:
            if (SDL_IsGameController(event.cdevice.which))
            {
                addController(event.cdevice.which);
            }
            else
            {
                addJoystick(event.cdevice.which);
            }
            return true;

        case SDL_CONTROLLERDEVICEREMOVED:
            removeController(event.cdevice.which);
            return true;

        case SDL_JOYDEVICEADDED:
            // Only handle if it's not a game controller (to avoid double-adding)
            if (!SDL_IsGameController(event.jdevice.which))
            {
                addJoystick(event.jdevice.which);
            }
            return true;

        case SDL_JOYDEVICEREMOVED:
            removeController(event.jdevice.which);
            return true;

        case SDL_CONTROLLERBUTTONDOWN:
        {
            LOG_DEBUG("Controller button pressed: %d", event.cbutton.button);
            GamepadButton button = static_cast<GamepadButton>(event.cbutton.button);
            updateButtonState(button, ButtonState::Pressed);

            // Call specific button callbacks
            int buttonInt = static_cast<int>(button);
            if (buttonPressedCallbacks_.count(buttonInt))
            {
                for (auto &callback : buttonPressedCallbacks_[buttonInt])
                {
                    callback();
                }
            }

            // Call general button event callbacks
            for (auto &callback : buttonEventCallbacks_)
            {
                callback(button, ButtonState::Pressed);
            }

            return true;
        }

        case SDL_CONTROLLERBUTTONUP:
        {
            LOG_DEBUG("Controller button released: %d", event.cbutton.button);
            GamepadButton button = static_cast<GamepadButton>(event.cbutton.button);
            updateButtonState(button, ButtonState::Released);

            // Call specific button callbacks
            int buttonInt = static_cast<int>(button);
            if (buttonReleasedCallbacks_.count(buttonInt))
            {
                for (auto &callback : buttonReleasedCallbacks_[buttonInt])
                {
                    callback();
                }
            }

            // Call general button event callbacks
            for (auto &callback : buttonEventCallbacks_)
            {
                callback(button, ButtonState::Released);
            }

            return true;
        }

        case SDL_CONTROLLERAXISMOTION:
        {
            GamepadAxis axis = static_cast<GamepadAxis>(event.caxis.axis);
            float value = event.caxis.value / 32768.0f; // Normalize to -1.0 to 1.0
            updateAxisValue(axis, value);

            // Call axis event callbacks
            for (auto &callback : axisEventCallbacks_)
            {
                callback(axis, value);
            }

            return true;
        }

        // Fallback: Handle joystick events if device isn't recognized as game controller
        case SDL_JOYBUTTONDOWN:
        {
            LOG_DEBUG("Joystick button pressed: %d", event.jbutton.button);
            // Map common joystick buttons to GamepadButton enum
            if (event.jbutton.button < 16) // Limit to reasonable button count
            {
                GamepadButton button = static_cast<GamepadButton>(event.jbutton.button);
                updateButtonState(button, ButtonState::Pressed);

                // Call callbacks
                int buttonInt = static_cast<int>(button);
                if (buttonPressedCallbacks_.count(buttonInt))
                {
                    for (auto &callback : buttonPressedCallbacks_[buttonInt])
                    {
                        callback();
                    }
                }

                for (auto &callback : buttonEventCallbacks_)
                {
                    callback(button, ButtonState::Pressed);
                }
            }
            return true;
        }

        case SDL_JOYBUTTONUP:
        {
            LOG_DEBUG("Joystick button released: %d", event.jbutton.button);
            // Map common joystick buttons to GamepadButton enum
            if (event.jbutton.button < 16) // Limit to reasonable button count
            {
                GamepadButton button = static_cast<GamepadButton>(event.jbutton.button);
                updateButtonState(button, ButtonState::Released);

                // Call callbacks
                int buttonInt = static_cast<int>(button);
                if (buttonReleasedCallbacks_.count(buttonInt))
                {
                    for (auto &callback : buttonReleasedCallbacks_[buttonInt])
                    {
                        callback();
                    }
                }

                for (auto &callback : buttonEventCallbacks_)
                {
                    callback(button, ButtonState::Released);
                }
            }
            return true;
        }

        default:
            return false;
        }
    }

    void ControllerManager::onButtonPressed(GamepadButton button, std::function<void()> callback)
    {
        int buttonInt = static_cast<int>(button);
        buttonPressedCallbacks_[buttonInt].push_back(callback);
    }

    void ControllerManager::onButtonReleased(GamepadButton button, std::function<void()> callback)
    {
        int buttonInt = static_cast<int>(button);
        buttonReleasedCallbacks_[buttonInt].push_back(callback);
    }

    void ControllerManager::onButtonEvent(ButtonCallback callback)
    {
        buttonEventCallbacks_.push_back(callback);
    }

    void ControllerManager::onAxisEvent(AxisCallback callback)
    {
        axisEventCallbacks_.push_back(callback);
    }

    bool ControllerManager::isButtonPressed(GamepadButton button) const
    {
        int buttonInt = static_cast<int>(button);
        auto it = buttonStates_.find(buttonInt);
        return it != buttonStates_.end() && it->second;
    }

    float ControllerManager::getAxisValue(GamepadAxis axis) const
    {
        int axisInt = static_cast<int>(axis);
        auto it = axisValues_.find(axisInt);
        return it != axisValues_.end() ? it->second : 0.0f;
    }

    void ControllerManager::addController(int deviceIndex)
    {
        SDL_GameController *controller = SDL_GameControllerOpen(deviceIndex);
        if (!controller)
        {
            LOG_ERROR("Failed to open game controller %d: %s", deviceIndex, SDL_GetError());
            return;
        }

        SDL_Joystick *joystick = SDL_GameControllerGetJoystick(controller);
        SDL_JoystickID instanceId = SDL_JoystickInstanceID(joystick);
        const char *name = SDL_GameControllerName(controller);

        auto controllerInfo = std::make_unique<ControllerInfo>();
        controllerInfo->controller = controller;
        controllerInfo->instanceId = instanceId;
        controllerInfo->name = name ? name : "Unknown Controller";
        controllerInfo->connected = true;
        controllerInfo->isGameController = true;

        controllers_.push_back(std::move(controllerInfo));

        LOG_INFO("Controller connected: %s (ID: %d)", name ? name : "Unknown", instanceId);
    }

    void ControllerManager::addJoystick(int deviceIndex)
    {
        SDL_Joystick *joystick = SDL_JoystickOpen(deviceIndex);
        if (!joystick)
        {
            LOG_ERROR("Failed to open joystick %d: %s", deviceIndex, SDL_GetError());
            return;
        }

        SDL_JoystickID instanceId = SDL_JoystickInstanceID(joystick);
        const char *name = SDL_JoystickName(joystick);

        auto controllerInfo = std::make_unique<ControllerInfo>();
        controllerInfo->joystick = joystick;
        controllerInfo->controller = nullptr;
        controllerInfo->instanceId = instanceId;
        controllerInfo->name = name ? name : "Unknown Joystick";
        controllerInfo->connected = true;
        controllerInfo->isGameController = false;

        controllers_.push_back(std::move(controllerInfo));

        LOG_INFO("Joystick connected: %s (ID: %d)", name ? name : "Unknown", instanceId);
    }

    void ControllerManager::removeController(SDL_JoystickID instanceId)
    {
        auto it = std::find_if(controllers_.begin(), controllers_.end(), [instanceId](const std::unique_ptr<ControllerInfo> &info)
                               { return info->instanceId == instanceId; });

        if (it != controllers_.end())
        {
            LOG_INFO("Controller/Joystick disconnected: %s (ID: %d)", (*it)->name.c_str(), instanceId);

            if ((*it)->controller)
            {
                SDL_GameControllerClose((*it)->controller);
            }
            else if ((*it)->joystick)
            {
                SDL_JoystickClose((*it)->joystick);
            }

            controllers_.erase(it);
        }
    }

    ControllerManager::ControllerInfo *ControllerManager::findController(SDL_JoystickID instanceId)
    {
        auto it = std::find_if(controllers_.begin(), controllers_.end(), [instanceId](const std::unique_ptr<ControllerInfo> &info)
                               { return info->instanceId == instanceId; });

        return it != controllers_.end() ? it->get() : nullptr;
    }

    void ControllerManager::updateButtonState(GamepadButton button, ButtonState state)
    {
        int buttonInt = static_cast<int>(button);
        buttonStates_[buttonInt] = (state == ButtonState::Pressed);
    }

    void ControllerManager::updateAxisValue(GamepadAxis axis, float value)
    {
        int axisInt = static_cast<int>(axis);
        axisValues_[axisInt] = value;
    }

} // namespace TG5040
