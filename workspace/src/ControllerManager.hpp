#pragma once

#include <SDL2/SDL.h>
#include <functional>
#include <unordered_map>
#include <vector>
#include <memory>

namespace TG5040
{

    // Controller button mapping using SDL enums
    enum class GamepadButton
    {
        A = SDL_CONTROLLER_BUTTON_A,
        B = SDL_CONTROLLER_BUTTON_B,
        X = SDL_CONTROLLER_BUTTON_X,
        Y = SDL_CONTROLLER_BUTTON_Y,
        BACK = SDL_CONTROLLER_BUTTON_BACK,
        GUIDE = SDL_CONTROLLER_BUTTON_GUIDE,
        START = SDL_CONTROLLER_BUTTON_START,
        LEFT_STICK = SDL_CONTROLLER_BUTTON_LEFTSTICK,
        RIGHT_STICK = SDL_CONTROLLER_BUTTON_RIGHTSTICK,
        LEFT_SHOULDER = SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
        RIGHT_SHOULDER = SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
        DPAD_UP = SDL_CONTROLLER_BUTTON_DPAD_UP,
        DPAD_DOWN = SDL_CONTROLLER_BUTTON_DPAD_DOWN,
        DPAD_LEFT = SDL_CONTROLLER_BUTTON_DPAD_LEFT,
        DPAD_RIGHT = SDL_CONTROLLER_BUTTON_DPAD_RIGHT
    };

    // Controller axis mapping
    enum class GamepadAxis
    {
        LEFT_X = SDL_CONTROLLER_AXIS_LEFTX,
        LEFT_Y = SDL_CONTROLLER_AXIS_LEFTY,
        RIGHT_X = SDL_CONTROLLER_AXIS_RIGHTX,
        RIGHT_Y = SDL_CONTROLLER_AXIS_RIGHTY,
        LEFT_TRIGGER = SDL_CONTROLLER_AXIS_TRIGGERLEFT,
        RIGHT_TRIGGER = SDL_CONTROLLER_AXIS_TRIGGERRIGHT
    };

    // Button state
    enum class ButtonState
    {
        Pressed,
        Released
    };

    // Event callbacks
    using ButtonCallback = std::function<void(GamepadButton button, ButtonState state)>;
    using AxisCallback = std::function<void(GamepadAxis axis, float value)>;

    class ControllerManager
    {
    public:
        // Singleton pattern
        static ControllerManager &getInstance();

        ControllerManager(const ControllerManager &) = delete;
        ControllerManager &operator=(const ControllerManager &) = delete;

        // Initialize controller system
        bool initialize();

        // Shutdown controller system
        void shutdown();

        // Process SDL events (call from main event loop)
        bool handleEvent(const SDL_Event &event);

        // Register callbacks for button events
        void onButtonPressed(GamepadButton button, std::function<void()> callback);
        void onButtonReleased(GamepadButton button, std::function<void()> callback);
        void onButtonEvent(ButtonCallback callback);

        // Register callbacks for axis events
        void onAxisEvent(AxisCallback callback);

        // Check current button state
        bool isButtonPressed(GamepadButton button) const;

        // Get current axis value (-1.0 to 1.0)
        float getAxisValue(GamepadAxis axis) const;

        // Get number of connected controllers
        int getControllerCount() const { return static_cast<int>(controllers_.size()); }

        // Check if any controller is connected
        bool hasController() const { return !controllers_.empty(); }

    private:
        ControllerManager() = default;
        ~ControllerManager() = default;

        struct ControllerInfo
        {
            SDL_GameController *controller = nullptr;
            SDL_JoystickID instanceId = -1;
            std::string name;
            bool connected = false;
        };

        std::vector<std::unique_ptr<ControllerInfo>> controllers_;
        std::unordered_map<int, bool> buttonStates_; // button -> pressed state
        std::unordered_map<int, float> axisValues_;  // axis -> value

        // Event callbacks
        std::unordered_map<int, std::vector<std::function<void()>>> buttonPressedCallbacks_;
        std::unordered_map<int, std::vector<std::function<void()>>> buttonReleasedCallbacks_;
        std::vector<ButtonCallback> buttonEventCallbacks_;
        std::vector<AxisCallback> axisEventCallbacks_;

        bool initialized_ = false;

        // Helper methods
        void addController(int deviceIndex);
        void removeController(SDL_JoystickID instanceId);
        ControllerInfo *findController(SDL_JoystickID instanceId);
        void updateButtonState(GamepadButton button, ButtonState state);
        void updateAxisValue(GamepadAxis axis, float value);
    };

} // namespace TG5040
