#!/usr/bin/env cjit

// this example shows how to use dynamic plugins without dlopen
// it uses libSDL2 and frei0r video plugins (apt-get install frei0r-plugins-dev)

// example usage: ./cjit frei0r_generator.c /usr/lib/frei0r-1/ising0r.so

#pragma comment(lib, "SDL2")
#define SDL_DISABLE_IMMINTRIN_H 1

#include <SDL2/SDL.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdint.h>
#include <frei0r.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

// Frei0r functions
extern void f0r_get_plugin_info(f0r_plugin_info_t* nois0rInfo);
extern f0r_instance_t f0r_construct(unsigned int width, unsigned int height);
extern void f0r_destruct(f0r_instance_t instance);
void f0r_update(f0r_instance_t instance, double time, const uint32_t* inframe, uint32_t* outframe);
extern void f0r_destruct(f0r_instance_t instance);

int main(int argc, char* argv[]) {
    if (!f0r_get_plugin_info || !f0r_construct || !f0r_update || !f0r_destruct) {
        fprintf(stderr, "Missing Frei0r plugin\n");
        exit(1);
    }

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    // Create SDL window and renderer
    SDL_Window* window = SDL_CreateWindow("Frei0r CJIT example",
                                          SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0x0);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                             SDL_TEXTUREACCESS_STREAMING,
                                             WINDOW_WIDTH, WINDOW_HEIGHT);

    // Create the Frei0r effect instance
    f0r_instance_t effect_instance = f0r_construct(WINDOW_WIDTH, WINDOW_HEIGHT);

    // Main loop
    int running = 1;
    SDL_Event event;
    void* pixels;
    int pitch;
    double time= 0;

    // Set the initial background color
    SDL_LockTexture(texture, NULL, &pixels, &pitch);
    uint32_t* pixel_ptr = (uint32_t*)pixels;
    for (int y = 0; y < WINDOW_HEIGHT; ++y) {
        for (int x = 0; x < WINDOW_WIDTH; ++x) {
            pixel_ptr[y * (pitch / 4) + x] = SDL_MapRGB(SDL_AllocFormat(SDL_PIXELFORMAT_RGB888), 50, 100, 150);
        }
    }
    SDL_UnlockTexture(texture);
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }
        time += 0.1;

        // Apply the Frei0r noise effect on the texture
        SDL_LockTexture(texture, NULL, &pixels, &pitch);
        f0r_update(effect_instance, time, NULL, pixels);  // Apply noise effect on pixels
        SDL_UnlockTexture(texture);

        // Render the updated texture
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

    }

    // Clean up
    f0r_destruct(effect_instance);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
