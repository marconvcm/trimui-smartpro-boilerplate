#include "SDLManager.hpp"
#include "Logger.hpp"

namespace TG5040
{
    SDLManager &SDLManager::getInstance()
    {
        static SDLManager instance;
        return instance;
    }

    bool SDLManager::initialize(int screenWidth, int screenHeight, const std::string &title)
    {
        if (initialized_)
        {
            LOG_WARN("SDL Manager already initialized");
            return true;
        }

        screenWidth_ = screenWidth;
        screenHeight_ = screenHeight;

        LOG_INFO("Initializing SDL...");

        // Initialize SDL
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO) < 0)
        {
            LOG_FATAL("SDL initialization failed: %s", SDL_GetError());
            return false;
        }

        // Initialize SDL_ttf
        if (TTF_Init() < 0)
        {
            LOG_FATAL("TTF initialization failed: %s", TTF_GetError());
            SDL_Quit();
            return false;
        }

        // Initialize SDL_image
        int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
        if (!(IMG_Init(imgFlags) & imgFlags))
        {
            LOG_WARN("SDL_image could not initialize! SDL_image Error: %s", IMG_GetError());
        }

        // Create window
        window_ = SDL_CreateWindow(
            title.c_str(),
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            screenWidth_, screenHeight_,
            SDL_WINDOW_SHOWN);

        if (!window_)
        {
            LOG_FATAL("Window creation failed: %s", SDL_GetError());
            shutdown();
            return false;
        }

        // Create renderer
        renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (!renderer_)
        {
            LOG_FATAL("Renderer creation failed: %s", SDL_GetError());
            shutdown();
            return false;
        }

        // Set renderer blend mode for alpha blending
        SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);

        initialized_ = true;
        LOG_INFO("SDL initialized successfully [%dx%d]", screenWidth_, screenHeight_);
        return true;
    }

    void SDLManager::shutdown()
    {
        clearFontCache();

        if (renderer_)
        {
            SDL_DestroyRenderer(renderer_);
            renderer_ = nullptr;
        }

        if (window_)
        {
            SDL_DestroyWindow(window_);
            window_ = nullptr;
        }

        IMG_Quit();
        TTF_Quit();
        SDL_Quit();

        initialized_ = false;
        LOG_INFO("SDL shutdown complete");
    }

    TTF_Font *SDLManager::loadFont(const std::string &fontPath, int fontSize)
    {
        std::string key = fontPath + ":" + std::to_string(fontSize);

        // Check if already cached
        auto it = fontCache_.find(key);
        if (it != fontCache_.end())
        {
            return it->second;
        }

        // Load new font
        TTF_Font *font = TTF_OpenFont(fontPath.c_str(), fontSize);
        if (!font)
        {
            LOG_ERROR("Failed to load font %s at size %d: %s", fontPath.c_str(), fontSize, TTF_GetError());
            return nullptr;
        }

        // Cache the font
        fontCache_[key] = font;
        LOG_INFO("Loaded font: %s at size %d", fontPath.c_str(), fontSize);

        return font;
    }

    TTF_Font *SDLManager::getFont(const std::string &fontPath, int fontSize)
    {
        return loadFont(fontPath, fontSize);
    }

    TTF_Font *SDLManager::getDefaultFont(int fontSize)
    {
        return loadFont(defaultFontPath_, fontSize);
    }

    void SDLManager::clearFontCache()
    {
        for (auto &pair : fontCache_)
        {
            if (pair.second)
            {
                TTF_CloseFont(pair.second);
            }
        }
        fontCache_.clear();
        LOG_INFO("Font cache cleared");
    }

} // namespace TG5040
