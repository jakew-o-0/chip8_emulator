#ifndef _CHIP8_
#define _CHIP8_

#include<fstream>
#include<iostream>
#include<sys/types.h>
#include<string>


class Chip8_CPU
{
private:
    unsigned char RAM[4096];       // 4KB memory
    unsigned char REG[16];         // general purpose registers 0-F, F is used as a carry flag
    uint16_t STACK[16];     // calling and returning from subroutines
    uint16_t SP;            // stack pointer
    uint16_t I;             // index register, holds a location in memory, often used for drawing to SCREEN
    uint16_t PC;            // program counter, holds next address to be interpreted
    unsigned char DELAY_TIMER;     // will count douwn while its value is greater than 0
    unsigned char SOUND_TIMER;     // will count down, making a noise, while value is greater than 0

    /*========= Memory map =========
        - 0x000 -> 0x1FF    chip8 interpreter( only necicary for character font )
        - 0x050 -> 0x0A0    character fonts location
        - 0x200 -> 0xFFF    program memory and work ram
    ================================*/


    // internal variables used accross functions
    uint16_t opcode;        // holds curent instruction to be decoded
    unsigned char regX;
    unsigned char regY;
    unsigned char cond_value;
    unsigned char op_subType;

public:
    unsigned char key;          // input key being pressed
    unsigned char SCREEN[64*32];
    unsigned char screen_cond_flag; // F -> draw screen, 0 -> clear screen


public:
    void initialise();
    void load_game(std::string path);
    void fetch();
    void decode();
    void debug();

private:
    void math_logic_group();
    void draw_to_screen();
    void E_group();
    void F_group();
};
#endif
