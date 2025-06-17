#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>
#include <time.h>
#include "logger.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

// Helper function for linear interpolation (lerp)
float lerp(float a, float b, float t)
{
	return a + t * (b - a);
}

int main(void)
{
	// Initialize logger
	logger_init(LOG_LEVEL_DEBUG);
	LOG_INFO("Hello World starting...");

	// Initialize SDL with both video and joystick subsystems
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
	{
		LOG_FATAL("SDL initialization failed: %s", SDL_GetError());
		return 1;
	}

	// Initialize SDL_ttf
	if (TTF_Init() < 0)
	{
		LOG_FATAL("TTF initialization failed: %s", TTF_GetError());
		SDL_Quit();
		return 1;
	}

	// Create window
	SDL_Window *window = SDL_CreateWindow(
		"Hello World",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		SCREEN_WIDTH, SCREEN_HEIGHT,
		SDL_WINDOW_SHOWN);

	if (!window)
	{
		LOG_FATAL("Window creation failed: %s", SDL_GetError());
		TTF_Quit();
		SDL_Quit();
		return 1;
	}

	// Create renderer
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer)
	{
		LOG_FATAL("Renderer creation failed: %s", SDL_GetError());
		SDL_DestroyWindow(window);
		TTF_Quit();
		SDL_Quit();
		return 1;
	}

	// Load font
	TTF_Font *font = TTF_OpenFont("res/aller.ttf", 36);
	if (!font)
	{
		LOG_WARN("Could not load font: %s", TTF_GetError());
		// Try fallback font
		font = TTF_OpenFont("/usr/share/fonts/truetype/freefont/FreeSans.ttf", 36);
		if (!font)
		{
			LOG_ERROR("Could not load fallback font either");
		}
	}

	// Main loop flag
	int quit = 0;
	SDL_Event event;

	// Text to display
	const char *instructionText = "Application will close when timer ends";

	// Text color
	SDL_Color textColor = {255, 255, 255, 255};

	// Countdown variables
	const int countdownSeconds = 10;
	float currentTime = countdownSeconds;
	Uint32 lastTime = SDL_GetTicks();

	// Animation variables
	float targetScale = 1.0f;
	float currentScale = 1.0f;
	float pulseAmount = 0.0f;

	// Color transition variables
	SDL_Color startColor = {50, 50, 255, 255}; // Blue
	SDL_Color endColor = {255, 50, 50, 255};   // Red
	SDL_Color currentColor = startColor;

	// Main loop
	while (!quit)
	{
		// Handle events - only check for quit event
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				quit = 1;
			}
		}

		// Calculate delta time
		Uint32 currentTicks = SDL_GetTicks();
		float deltaTime = (currentTicks - lastTime) / 1000.0f;
		lastTime = currentTicks;

		// Update countdown
		if (currentTime > 0)
		{
			currentTime -= deltaTime;
			if (currentTime <= 0)
			{
				currentTime = 0;
				LOG_INFO("Timer ended, exiting application");
				quit = 1; // Exit when timer reaches zero
			}

			// Calculate progress (0 = start, 1 = end)
			float progress = 1.0f - (currentTime / countdownSeconds);

			// Update color using lerp
			currentColor.r = lerp(startColor.r, endColor.r, progress);
			currentColor.g = lerp(startColor.g, endColor.g, progress);
			currentColor.b = lerp(startColor.b, endColor.b, progress);

			// Calculate pulse scale based on remaining time
			// More frequent pulses as time runs out
			float pulseSpeed = lerp(1.0f, 5.0f, progress);
			pulseAmount = (0.25f * sin(currentTicks * 0.003f * pulseSpeed)) + 1.0f;

			// Larger scale as time runs out
			if (currentTime < 3.0f)
			{
				targetScale = lerp(1.0f, 2.0f, 1.0f - (currentTime / 3.0f));
			}
		}

		// Smoothly interpolate current scale towards target
		currentScale = lerp(currentScale, targetScale * pulseAmount, deltaTime * 5.0f);

		// Gradually return target scale to normal
		targetScale = lerp(targetScale, 1.0f, deltaTime * 2.0f);

		// Clear screen - background gets redder as countdown proceeds
		float bgIntensity = 1.0f - (currentTime / countdownSeconds);
		SDL_SetRenderDrawColor(renderer,
							   (Uint8)(20 + bgIntensity * 20),
							   (Uint8)(20 - bgIntensity * 15),
							   (Uint8)(20 - bgIntensity * 15), 255);
		SDL_RenderClear(renderer);

		// Render text if font is available
		if (font)
		{
			// Main title
			SDL_Surface *titleSurface = TTF_RenderText_Solid(font, "Hello, TG5040 World!", textColor);
			if (titleSurface)
			{
				SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, titleSurface);
				if (texture)
				{
					SDL_Rect destRect = {
						(SCREEN_WIDTH - titleSurface->w) / 2,
						(SCREEN_HEIGHT / 4) - (titleSurface->h / 2), // Top quarter of screen
						titleSurface->w,
						titleSurface->h};
					SDL_RenderCopy(renderer, texture, NULL, &destRect);
					SDL_DestroyTexture(texture);
				}
				SDL_FreeSurface(titleSurface);
			}

			// Format countdown as integer or decimal based on remaining time
			char countdownText[32];
			if (currentTime >= 1.0f)
			{
				snprintf(countdownText, sizeof(countdownText), "%d", (int)ceil(currentTime));
			}
			else if (currentTime > 0.0f)
			{
				snprintf(countdownText, sizeof(countdownText), "%.1f", currentTime);
			}
			else
			{
				snprintf(countdownText, sizeof(countdownText), "Goodbye!");
			}

			// Countdown number
			SDL_Surface *countSurface = TTF_RenderText_Solid(font, countdownText, currentColor);
			if (countSurface)
			{
				SDL_Texture *countTexture = SDL_CreateTextureFromSurface(renderer, countSurface);
				if (countTexture)
				{
					// Calculate scaled width and height
					int scaledW = (int)(countSurface->w * currentScale);
					int scaledH = (int)(countSurface->h * currentScale);

					SDL_Rect countRect = {
						(SCREEN_WIDTH - scaledW) / 2,
						(SCREEN_HEIGHT - scaledH) / 2, // Center of screen
						scaledW,
						scaledH};
					SDL_RenderCopy(renderer, countTexture, NULL, &countRect);
					SDL_DestroyTexture(countTexture);
				}
				SDL_FreeSurface(countSurface);
			}

			// Button instructions (smaller font size)
			TTF_Font *smallFont = TTF_OpenFont("res/aller.ttf", 24);
			if (!smallFont)
				smallFont = TTF_OpenFont("/usr/share/fonts/truetype/freefont/FreeSans.ttf", 24);

			if (smallFont)
			{
				SDL_Color instructionColor = {200, 200, 200, 255};
				SDL_Surface *instructionSurface = TTF_RenderText_Solid(smallFont, instructionText, instructionColor);
				if (instructionSurface)
				{
					SDL_Texture *instructionTexture = SDL_CreateTextureFromSurface(renderer, instructionSurface);
					if (instructionTexture)
					{
						SDL_Rect instructRect = {
							(SCREEN_WIDTH - instructionSurface->w) / 2,
							SCREEN_HEIGHT - instructionSurface->h - 50, // Bottom of screen
							instructionSurface->w,
							instructionSurface->h};
						SDL_RenderCopy(renderer, instructionTexture, NULL, &instructRect);
						SDL_DestroyTexture(instructionTexture);
					}
					SDL_FreeSurface(instructionSurface);
				}
				TTF_CloseFont(smallFont);
			}
		}

		// Update screen
		SDL_RenderPresent(renderer);

		// Small delay to reduce CPU usage
		SDL_Delay(16);
	} // End of main loop

	// Cleanup
	if (font)
	{
		TTF_CloseFont(font);
	}
	
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	TTF_Quit();
	SDL_Quit();
	logger_close();

	return 0;
}
