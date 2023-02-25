#ifndef CPU_H
#define CPU_H

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
} CPU;

#endif