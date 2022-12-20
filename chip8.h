#ifndef _CHIP8_
#define _CHIP8_

#include<fstream>
#include<iostream>
#include<sys/types.h>
#include<vector>
#include<string>


class Chip8_CPU
{
private:
    u_char RAM[4096];       // 4KB memory
    u_char REG[16];         // general purpose registers 0-F, F is used as a carry flag
    uint16_t STACK[16];     // calling and returning from subroutines
    uint16_t SP;            // stack pointer
    uint16_t I;             // index register, holds a location in memory, often used for drawing to SCREEN
    uint16_t PC;            // program counter, holds next address to be interpreted
    u_char DELAY_TIMER;     // will count douwn while its value is greater than 0
    u_char SOUND_TIMER;     // will count down, making a noise, while value is greater than 0
    u_char SCREEN[64 * 32]; // 64x23 pixel screen, 0x00 -> off and 0xFF -> on

    /*========= Memory map =========
        - 0x000 -> 0x1FF    chip8 interpreter( only necicary for character font )
        - 0x050 -> 0x0A0    character fonts location
        - 0x200 -> 0xFFF    program memory and work ram
    ================================*/


    uint16_t opcode;        // holds curent instruction to be decoded
    u_char regX;
    u_char regY;
    u_char cond_value;


public:
    void initialise();
    void load_game(std::string path);
    void fetch();
    void decode();

private:
    void math_logic_group();
    void draw_to_screen();
    void E_group();
    void F_group();
};
#endif
