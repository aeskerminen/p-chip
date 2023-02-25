#include "cpu.h"

void CPU::tick()
{
    uint16_t op = (memory[PC] << 8) | memory[PC + 1];
    std::cout
        << "HI: " << (uint16_t)memory[PC] << " | LO:" << (uint16_t)memory[PC + 1] << "| OPCODE: " << op << " | PC: " << PC << "\n";

    PC += 2;

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
                screen[i] = 0;
            draw = true;
            break;
        case 0x000E:
            SP--;
            PC = stack[SP];
            break;
        }
        break;
    }
    case 0x1000:
        PC = (nnn);
        break;
    case 0x2000:
        stack[SP] = PC;
        ++SP;
        PC = (nnn);
        break;
    case 0x3000:
        if (V[x] == (nn))
            PC += 2;
        break;
    case 0x4000:
        if (V[x] != (nn))
            PC += 2;
        break;
    case 0x5000:
        if (V[x] == V[y])
            PC += 2;
        break;
    case 0x6000:
        V[x] = (nn);
        break;
    case 0x7000:
        V[x] += (nn);
        break;

    case 0x8000:
    {
        switch (op & 0x000F)
        {
        case 0x0000:
            V[x] = V[y];
            break;
        case 0x0001:
            V[x] |= V[y];
            break;
        case 0x0002:
            V[x] &= V[y];
            break;
        case 0x0003:
            V[x] ^= V[y];
            break;
        case 0x0004:
            V[x] += V[y];

            if (V[y] > (0xFF - V[x]))
                V[0xF] = 1;
            else
                V[0xF] = 0;
            break;
        case 0x0005:
            if (V[y] > V[x])
                V[0xF] = 0;
            else
                V[0xF] = 1;

            V[x] -= V[y];
            break;
        case 0x0006:
            V[0xF] = V[x] & 0x1;
            V[x] >>= 1;
            break;
        case 0x0007:
            if (V[x] > V[y])
                V[0xF] = 0;
            else
                V[0xF] = 1;

            V[x] = V[y] - V[x];
            break;
        case 0x000E:
            V[0xF] = V[x] >> 7;
            V[x] <<= 1;
            break;
        }
        break;
    }

    case 0x9000:
        if (V[x] != V[y])
            PC += 2;
        break;
    case 0xA000:
        I = nnn;
        break;
    case 0xB000:
        PC = V[0x0] + (nnn);
        break;
    case 0xC000:
        V[x] = (rand() % (0xFF + 1)) & (nn);
        break;
    case 0xD000:
    {
        unsigned short cx = V[x];
        unsigned short cy = V[y];
        unsigned short height = op & 0x000F;
        unsigned short pixel;

        V[0xF] = 0;
        for (int yline = 0; yline < height; yline++)
        {
            pixel = memory[I + yline];
            for (int xline = 0; xline < 8; xline++)
            {
                if ((pixel & (0x80 >> xline)) != 0)
                {
                    int coordinate = ((cx + xline) + ((cy + yline) * 64)) % 2048;
                    if (screen[coordinate] == 1)
                        V[0xF] = 1;

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
            if (keyboard[V[x]] != 0)
                PC += 2;
            break;
        case 0x00A1:
            if (keyboard[V[x]] == 0)
                PC += 2;
            break;
        }
        break;
    }

    case 0xF000:
    {
        switch ((nn))
        {
        case 0x0007:
            V[x] = DT;
            break;
        case 0x000A:
        {
            bool found = false;
            for (int i = 0; i < 16; i++)
            {
                if (keyboard[i] != 0)
                {
                    V[x] = i;
                    found = true;
                }
            }

            if (!found)
            {
                PC -= 2;
                return;
            }
        }
        break;
        case 0x0015:
            DT = V[x];
            break;
        case 0x0018:
            ST = V[x];
            break;
        case 0x001E:
            if (I + V[x] > 0xFFF)
                V[0xF] = 1;
            else
                V[0xF] = 0;
            I += V[x];
            break;
        case 0x0029:
            I = V[x] * 0x5;
            break;
        case 0x0033:
        {
            auto num = V[x];

            memory[I] = (num / 100);
            memory[I + 1] = (num / 10) % 10;
            memory[I + 2] = num % 10;
        }
        break;
        case 0x0055:
        {
            for (int i = 0; i <= (x); i++)
                memory[I + i] = V[i];
        }
        break;
        case 0x0065:
        {
            for (int i = 0; i <= (x); i++)
                V[i] = memory[I + i];
        }
        break;
        }
        break;
    }
    }
}