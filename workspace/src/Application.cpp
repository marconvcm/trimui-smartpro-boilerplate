#include "Application.hpp"
#include "Logger.hpp"

namespace TG5040
{

    Application::Application(const std::string &title, int width, int height)
        : title_(title), width_(width), height_(height)
    {
    }

    Application::~Application()
    {
        shutdown();
    }

    bool Application::initialize()
    {
        // Initialize logger
        Logger::getInstance().init();
        LOG_INFO("Starting TG5040 Application: %s", title_.c_str());

        // Initialize SDL
        if (!SDLManager::getInstance().initialize(width_, height_, title_))
        {
            LOG_FATAL("Failed to initialize SDL Manager");
            return false;
        }

        // Initialize controller manager
        if (!ControllerManager::getInstance().initialize())
        {
            LOG_WARN("Failed to initialize Controller Manager");
            // Not fatal, continue without controller support
        }

        // Call user initialization
        onCreate();

        return true;
    }

    void Application::run()
    {
        if (!SDLManager::getInstance().isInitialized())
        {
            LOG_ERROR("Cannot run application - SDL not initialized");
            return;
        }

        running_ = true;
        lastTime_ = SDL_GetTicks();

        LOG_INFO("Application main loop started");

        while (running_)
        {
            frameStart_ = SDL_GetTicks();

            calculateDeltaTime();
            handleEvents();
            update();
            render();

            // Limit frame rate to 60 FPS
            limitFrameRate();
        }

        LOG_INFO("Application main loop ended");
    }

    void Application::shutdown()
    {
        if (running_)
        {
            running_ = false;
        }

        rootElement_.reset();
        ControllerManager::getInstance().shutdown();
        SDLManager::getInstance().shutdown();
        Logger::getInstance().close();
    }

    void Application::calculateDeltaTime()
    {
        Uint32 currentTime = SDL_GetTicks();
        deltaTime_ = (currentTime - lastTime_) / 1000.0f;

        // Cap delta time to prevent large jumps (e.g., when debugging or pausing)
        constexpr float MAX_DELTA_TIME = 1.0f / 30.0f; // Cap at 30 FPS equivalent
        if (deltaTime_ > MAX_DELTA_TIME)
        {
            deltaTime_ = MAX_DELTA_TIME;
        }

        lastTime_ = currentTime;
    }

    void Application::handleEvents()
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            // Debug: Log all SDL events
            if (event.type >= SDL_FIRSTEVENT && event.type <= SDL_LASTEVENT)
            {
                LOG_DEBUG("SDL Event received: type=%d", event.type);
            }
            
            // Check for quit event
            if (event.type == SDL_QUIT)
            {
                quit();
                continue;
            }

            // Handle controller events first
            bool controllerHandled = ControllerManager::getInstance().handleEvent(event);
            if (controllerHandled)
            {
                continue;
            }

            // Handle UI events
            bool handled = false;
            if (rootElement_)
            {
                handled = rootElement_->handleEvent(event);
            }

            // If UI didn't handle the event, pass it to user code
            if (!handled)
            {
                handled = onEvent(event);
            }

            // Handle default key events
            if (!handled && event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    quit();
                }
            }
        }
    }

    void Application::update()
    {
        // Call user update
        onUpdate(deltaTime_);

        // Update UI layout if needed
        if (rootElement_)
        {
            if (rootElement_->needsLayout())
            {
                // Set root frame to screen size
                rootElement_->frame = UI::Rect(0, 0, width_, height_);
                rootElement_->layoutSubviews();
            }
        }
    }

    void Application::render()
    {
        SDL_Renderer *renderer = SDLManager::getInstance().getRenderer();

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);

        // Render UI
        if (rootElement_)
        {
            rootElement_->render(renderer);
        }

        // Call user render
        onRender();

        // Present the frame
        SDL_RenderPresent(renderer);
    }

    void Application::limitFrameRate()
    {
        Uint32 frameTime = SDL_GetTicks() - frameStart_;

        if (frameTime < TARGET_FRAME_TIME)
        {
            // Calculate delay needed to maintain target frame rate
            Uint32 delayTime = static_cast<Uint32>(TARGET_FRAME_TIME - frameTime);
            SDL_Delay(delayTime);
        }
    }

} // namespace TG5040
