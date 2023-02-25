// CHIP8
#include <iostream>
#include <SDL2/SDL.h>

#include <fstream>
#include <iterator>
#include <vector>

#include <random>

#include <chrono>
#include <thread>

#include <Windows.h>

#include "cpu.h"

const unsigned char font[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

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

CPU cpu;

bool draw = false;

int main(int argv, char *args[])
{
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Renderer *renderer;
    SDL_Window *window;

    int w = 1024;
    int h = 512;

    window = SDL_CreateWindow("P-CHIP", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, 0);

    SDL_RenderSetLogicalSize(renderer, w, h);

    srand(time(NULL));

    for (int i = 0; i < 16; i++)
        cpu.V[i] = 0;
    for (int i = 0; i < 4096; i++)
        cpu.memory[i] = 0;
    for (int i = 0; i < 24; i++)
        cpu.stack[i] = 0;
    for (int i = 0; i < 2048; i++)
        cpu.screen[i] = 0;

    for (int i = 0; i < 16 * 5; i++)
    {
        cpu.memory[i] = font[i];
    }

    FILE *rom = fopen("./roms/games/pong.ch8", "rb");
    if (rom == NULL)
    {
        std::cerr << "Failed to open ROM" << std::endl;
        return false;
    }

    // Get file size
    fseek(rom, 0, SEEK_END);
    long rom_size = ftell(rom);
    rewind(rom);

    // Allocate memory to store rom
    char *rom_buffer = (char *)malloc(sizeof(char) * rom_size);
    if (rom_buffer == NULL)
    {
        std::cerr << "Failed to allocate memory for ROM" << std::endl;
        return false;
    }

    // Copy ROM into buffer
    size_t result = fread(rom_buffer, sizeof(char), (size_t)rom_size, rom);
    if (result != rom_size)
    {
        std::cerr << "Failed to read ROM" << std::endl;
        return false;
    }

    // Copy buffer to memory
    if ((4096 - 512) > rom_size)
    {
        for (int i = 0; i < rom_size; ++i)
        {
            cpu.memory[i + 512] = (uint8_t)rom_buffer[i];
        }
    }
    else
    {
        std::cerr << "ROM too large to fit in memory" << std::endl;
    }

    // Clean up
    fclose(rom);
    free(rom_buffer);

    SDL_Event e;

    bool play = true;
    SDL_Texture *sdlTexture = SDL_CreateTexture(renderer,
                                                SDL_PIXELFORMAT_ARGB8888,
                                                SDL_TEXTUREACCESS_STREAMING,
                                                64, 32);

    uint32_t pixels[2048];

    int cycles{0};

    while (play)
    {

        auto start_time = std::chrono::high_resolution_clock::now();

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

        cpu.tick();
        cycles++;

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
            cpu.draw = false;
            // Store pixels in temporary buffer
            for (int i = 0; i < 2048; ++i)
            {
                uint8_t pixel = cpu.screen[i];
                pixels[i] = (0x00FFFFFF * pixel) | 0xFF000000;
            }
            // Update SDL texture
            SDL_UpdateTexture(sdlTexture, NULL, pixels, 64 * sizeof(Uint32));
            // Clear screen and render
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, sdlTexture, NULL, NULL);
            SDL_RenderPresent(renderer);
        }

        Sleep(1);
    }

    return 0;
}
