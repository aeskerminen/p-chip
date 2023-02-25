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

void tick()
{
    uint16_t op = (cpu.memory[cpu.PC] << 8) | cpu.memory[cpu.PC + 1];
    std::cout
        << "HI: " << (uint16_t)memory[PC] << " | LO:" << (uint16_t)memory[PC + 1] << "| OPCODE: " << op << " | PC: " << PC << "\n";

    cpu.PC += 2;

    int x = ((op & 0x0F00) >> 8);
    int y = ((op & 0x00F0) >> 4);
    int nnn = (op & 0x0FFF);
    int nn = (op & 0x00FF);

    switch (op & 0xF000)
    {
    case 0x0000:
    {
        switch (op & 0x000F)
        {
        case 0x0000:
            for (int i = 0; i < 2048; i++)
                cpu.screen[i] = 0;
            draw = true;
            break;
        case 0x000E:
            cpu.cSP--;
            cpu.PC = cpu.stack[cpu.SP];
            break;
        }
        break;
    }
    case 0x1000:
        cpu.PC = (nnn);
        break;
    case 0x2000:
        stack[cpu.SP] = cpu.PC;
        ++cpu.SP;
        cpu.PC = (nnn);
        break;
    case 0x3000:
        if (cpu.V[x] == (nn))
            cpu.PC += 2;
        break;
    case 0x4000:
        if (cpu.V[x] != (nn))
            cpu.PC += 2;
        break;
    case 0x5000:
        if (cpu.V[x] == cpu.V[y])
            cpu.PC += 2;
        break;
    case 0x6000:
        cpu.V[x] = (nn);
        break;
    case 0x7000:
        cpu.V[x] += (nn);
        break;

    case 0x8000:
    {
        switch (op & 0x000F)
        {
        case 0x0000:
            cpu.V[x] = cpu.V[y];
            break;
        case 0x0001:
            cpu.V[x] |= cpu.V[y];
            break;
        case 0x0002:
            cpu.V[x] &= cpu.V[y];
            break;
        case 0x0003:
            cpu.V[x] ^= cpu.V[y];
            break;
        case 0x0004:
            cpu.V[x] += cpu.V[y];

            if (cpu.V[y] > (0xFF - cpu.V[x]))
                cpu.V[0xF] = 1;
            else
                cpu.V[0xF] = 0;
            break;
        case 0x0005:
            if (cpu.V[y] > cpu.V[x])
                cpu.V[0xF] = 0;
            else
                cpu.V[0xF] = 1;

            cpu.V[x] -= cpu.V[y];
            break;
        case 0x0006:
            cpu.V[0xF] = cpu.V[x] & 0x1;
            cpu.V[x] >>= 1;
            break;
        case 0x0007:
            if (cpu.V[x] > cpu.V[y])
                cpu.V[0xF] = 0;
            else
                cpu.V[0xF] = 1;

            cpu.V[x] = cpu.V[y] - cpu.V[x];
            break;
        case 0x000E:
            cpu.V[0xF] = cpu.V[x] >> 7;
            cpu.V[x] <<= 1;
            break;
        }
        break;
    }

    case 0x9000:
        if (cpu.V[x] != cpu.V[y])
            cpu.PC += 2;
        break;
    case 0xA000:
        cpu.I = nnn;
        break;
    case 0xB000:
        cpu.PC = cpu.V[0x0] + (nnn);
        break;
    case 0xC000:
        cpu.V[x] = (rand() % (0xFF + 1)) & (nn);
        break;
    case 0xD000:
    {
        unsigned short cx = cpu.V[x];
        unsigned short cy = cpu.V[y];
        unsigned short height = op & 0x000F;
        unsigned short pixel;

        cpu.V[0xF] = 0;
        for (int yline = 0; yline < height; yline++)
        {
            pixel = memory[I + yline];
            for (int xline = 0; xline < 8; xline++)
            {
                if ((pixel & (0x80 >> xline)) != 0)
                {
                    int coordinate = ((cx + xline) + ((cy + yline) * 64)) % 2048;
                    if (screen[coordinate] == 1)
                        cpu.V[0xF] = 1;

                    screen[coordinate] ^= 1;
                }
            }
        }

        draw = true;
    }
    break;
    case 0xE000:
    {
        switch ((nn))
        {
        case 0x009E:
            if (keyboard[cpu.V[x]] != 0)
                cpu.PC += 2;
            break;
        case 0x00A1:
            if (keyboard[cpu.V[x]] == 0)
                cpu.PC += 2;
            break;
        }
        break;
    }

    case 0xF000:
    {
        switch ((nn))
        {
        case 0x0007:
            cpu.V[x] = cpu.DT;
            break;
        case 0x000A:
        {
            bool found = false;
            for (int i = 0; i < 16; i++)
            {
                if (keyboard[i] != 0)
                {
                    cpu.V[x] = i;
                    found = true;
                }
            }

            if (!found)
            {
                cpu.PC -= 2;
                return;
            }
        }
        break;
        case 0x0015:
            cpu.DT = cpu.V[x];
            break;
        case 0x0018:
            cpu.ST = cpu.V[x];
            break;
        case 0x001E:
            if (cpu.I + cpu.V[x] > 0xFFF)
                cpu.V[0xF] = 1;
            else
                cpu.V[0xF] = 0;
            cpu.I += cpu.V[x];
            break;
        case 0x0029:
            cpu.I = cpu.V[x] * 0x5;
            break;
        case 0x0033:
        {
            auto num = cpu.V[x];

            memory[cpu.I] = (num / 100);
            memory[cpu.I + 1] = (num / 10) % 10;
            memory[cpu.I + 2] = num % 10;
        }
        break;
        case 0x0055:
        {
            for (int i = 0; i <= (x); i++)
                memory[cpu.I + i] = cpu.V[i];
        }
        break;
        case 0x0065:
        {
            for (int i = 0; i <= (x); i++)
                cpu.V[i] = memory[cpu.I + i];
        }
        break;
        }
        break;
    }
    }
}

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
        V[i] = 0;
    for (int i = 0; i < 4096; i++)
        memory[i] = 0;
    for (int i = 0; i < 24; i++)
        stack[i] = 0;
    for (int i = 0; i < 2048; i++)
        screen[i] = 0;

    for (int i = 0; i < 16 * 5; i++)
    {
        memory[i] = font[i];
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
            memory[i + 512] = (uint8_t)rom_buffer[i];
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
                        keyboard[i] = 1;
                    }
                }
            }

            if (e.type == SDL_KEYUP)
            {
                for (int i = 0; i < 16; ++i)
                {
                    if (e.key.keysym.sym == keymap[i])
                    {
                        keyboard[i] = 0;
                    }
                }
            }
        }

        tick();
        cycles++;

        if (cycles == 8)
        {
            if (ST > 0)
                ST--;
            if (DT > 0)
                DT--;
            cycles = 0;
        }

        if (draw)
        {
            draw = false;
            // Store pixels in temporary buffer
            for (int i = 0; i < 2048; ++i)
            {
                uint8_t pixel = screen[i];
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
