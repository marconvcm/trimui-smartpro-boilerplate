#pragma once

#include "SDLManager.hpp"
#include "ControllerManager.hpp"
#include "ConstraintLayout.hpp"
#include <SDL2/SDL.h>
#include <memory>

namespace TG5040
{

    class Application
    {
    public:
        Application(const std::string &title = "TG5040 App", int width = 1280, int height = 720);
        virtual ~Application();

        bool initialize();
        void run();
        void shutdown();

        // Override these in your application
        virtual void onCreate() {}
        virtual void onUpdate(float deltaTime) {}
        virtual void onRender() {}
        virtual bool onEvent(const SDL_Event &event) { return false; }

        // UI Management
        void setRootElement(UI::ElementPtr element) { rootElement_ = element; }
        UI::ElementPtr getRootElement() const { return rootElement_; }

        // Get delta time in seconds
        float getDeltaTime() const { return deltaTime_; }

        // Get current FPS
        float getFPS() const { return deltaTime_ > 0 ? 1.0f / deltaTime_ : 0.0f; }

        void quit() { running_ = false; }

    protected:
        std::string title_;
        int width_, height_;
        bool running_ = false;
        UI::ElementPtr rootElement_;

        Uint32 lastTime_ = 0;
        float deltaTime_ = 0.0f;

        // Frame rate control
        static constexpr int TARGET_FPS = 60;
        static constexpr float TARGET_FRAME_TIME = 1000.0f / TARGET_FPS; // milliseconds per frame
        Uint32 frameStart_ = 0;

        void calculateDeltaTime();
        void handleEvents();
        void update();
        void render();
        void limitFrameRate();
    };

} // namespace TG5040
