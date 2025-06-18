#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <memory>
#include <string>
#include <unordered_map>

namespace TG5040
{

    class SDLManager
    {
    public:
        static SDLManager &getInstance();

        bool initialize(int screenWidth = 1280, int screenHeight = 720, const std::string &title = "TG5040 App");
        void shutdown();

        SDL_Renderer *getRenderer() const { return renderer_; }
        SDL_Window *getWindow() const { return window_; }

        int getScreenWidth() const { return screenWidth_; }
        int getScreenHeight() const { return screenHeight_; }

        bool isInitialized() const { return initialized_; }

        // Font management
        TTF_Font *loadFont(const std::string &fontPath, int fontSize);
        TTF_Font *getFont(const std::string &fontPath, int fontSize);
        TTF_Font *getDefaultFont(int fontSize);
        void setDefaultFontPath(const std::string &fontPath) { defaultFontPath_ = fontPath; }

        // Prevent copying
        SDLManager(const SDLManager &) = delete;
        SDLManager &operator=(const SDLManager &) = delete;

    private:
        SDLManager() = default;
        ~SDLManager() = default;

        SDL_Window *window_ = nullptr;
        SDL_Renderer *renderer_ = nullptr;
        int screenWidth_ = 1280;
        int screenHeight_ = 720;
        bool initialized_ = false;

        // Font cache - key is "fontpath:size"
        std::unordered_map<std::string, TTF_Font *> fontCache_;
        std::string defaultFontPath_ = "res/aller.ttf";

        void clearFontCache();
    };

} // namespace TG5040
