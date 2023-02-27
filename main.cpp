// CHIP8
#include <SDL2/SDL.h>

#include <iostream>
#include <Windows.h>
#include <random>

#include <fstream>
#include <iterator>
#include <vector>

#include "cpu.h"

uint8_t keymap[16] = {
    SDLK_x,
    SDLK_1,
    SDLK_2,
    SDLK_3,
    SDLK_q,
    SDLK_w,
    SDLK_e,
    SDLK_a,
    SDLK_s,
    SDLK_d,
    SDLK_z,
    SDLK_c,
    SDLK_4,
    SDLK_r,
    SDLK_f,
    SDLK_v,
};

constexpr int w{1024};
constexpr int h{512};

SDL_Renderer *renderer{nullptr};
SDL_Window *window{nullptr};

void init();

int main(int argc, char *argv[])
{
    if (argc != 2)
        return 0;

    init();

    CPU cpu;
    uint32_t screenBuffer[2048];

    SDL_Event e;
    SDL_Texture *screenTexture = SDL_CreateTexture(renderer,
                                                   SDL_PIXELFORMAT_ARGB8888,
                                                   SDL_TEXTUREACCESS_STREAMING,
                                                   64, 32);
    int cycles{0};
    bool play{true};

    cpu.init();
    cpu.load_rom(argv[1]);

    while (play)
    {
        cpu.tick();
        cycles++;

        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT || e.key.keysym.sym == SDLK_ESCAPE)
                play = false;

            if (e.type == SDL_KEYDOWN)
            {
                for (int i = 0; i < 16; ++i)
                {
                    if (e.key.keysym.sym == keymap[i])
                    {
                        cpu.keyboard[i] = 1;
                    }
                }
            }

            if (e.type == SDL_KEYUP)
            {
                for (int i = 0; i < 16; ++i)
                {
                    if (e.key.keysym.sym == keymap[i])
                    {
                        cpu.keyboard[i] = 0;
                    }
                }
            }
        }

        if (cycles == 8)
        {
            if (cpu.ST > 0)
                cpu.ST--;
            if (cpu.DT > 0)
                cpu.DT--;
            cycles = 0;
        }

        if (cpu.draw)
        {
            for (int i = 0; i < 2048; ++i)
                screenBuffer[i] = (0x00FFFFFF * cpu.screen[i]);

            SDL_UpdateTexture(screenTexture, NULL, screenBuffer, 64 * sizeof(Uint32));

            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, screenTexture, NULL, NULL);
            SDL_RenderPresent(renderer);

            cpu.draw = false;
        }

        Sleep(1);
    }

    SDL_Quit();

    return 0;
}

void init()
{
    SDL_Init(SDL_INIT_EVERYTHING);

    window = SDL_CreateWindow("P-CHIP", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, 0);

    SDL_RenderSetLogicalSize(renderer, w, h);

    srand(time(NULL));
}
