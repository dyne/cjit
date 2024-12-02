#!/usr/bin/env cjit
#pragma comment(lib, "SDL2")
#define SDL_DISABLE_IMMINTRIN_H 1
#define SDL_MAIN_HANDLED 1

#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

int main(int argc, char **argv) {
	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
		return 1;
	}

	// Create SDL window and renderer
	SDL_Window* window = SDL_CreateWindow("SDL Random Noise",
					      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
					      WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888,
						 SDL_TEXTUREACCESS_STREAMING,
						 WINDOW_WIDTH, WINDOW_HEIGHT);

	int running = 1;
	SDL_Event event;

	while (running) {
		// Handle events
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				running = 0;
			}
		}

		// Fill the texture with random noise
		void* pixels;
		int pitch;
		SDL_LockTexture(texture, NULL, &pixels, &pitch);
		uint32_t* pixel_ptr = (uint32_t*)pixels;
		for (int y = 0; y < WINDOW_HEIGHT; ++y) {
			for (int x = 0; x < WINDOW_WIDTH; ++x) {
				// Generate random color
				uint8_t r = rand() % 256;
				uint8_t g = rand() % 256;
				uint8_t b = rand() % 256;
				pixel_ptr[y * (pitch / 4) + x] = SDL_MapRGB(SDL_AllocFormat(SDL_PIXELFORMAT_RGB888), r, g, b);
			}
		}
		SDL_UnlockTexture(texture);

		// Render the texture to the window
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
	}

	// Cleanup
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
   }
