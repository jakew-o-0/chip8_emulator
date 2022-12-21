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


    uint16_t opcode;        // holds curent instruction to be decoded
    unsigned char regX;
    unsigned char regY;
    unsigned char cond_value;
    unsigned char op_subType;

public:
    unsigned char key;
    unsigned char screen_cond;  // 0x00 no changes, 0x01 draw to screen, 0x02 clear screen
    unsigned char x_pos;
    unsigned char y_pos;
    std::vector<std::string> sprite;


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
