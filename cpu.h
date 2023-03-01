#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <random>

#include <iostream>
#include <fstream>

#include <Windows.h>

#include "font.h"

typedef struct CPU
{
    uint8_t V[16];
    uint8_t memory[4096];
    uint8_t screen[64 * 32];

    uint16_t SP;
    uint16_t stack[16];

    uint16_t PC;
    uint16_t I;

    uint8_t keyboard[16];

    uint8_t DT;
    uint8_t ST;

    bool draw;

    void init();
    void tick();
    void load_rom(const char *PATH);
} CPU;

#endif