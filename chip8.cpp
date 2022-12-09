#include<fstream>
#include <iostream>
#include <ostream>
#include<vector>
#include<string>

u_char RAM[4096];       // 4KB memory
u_char REG[16];         // general purpose registers 0-F, F is used as a carry flag
uint16_t STACK[16];     // calling and returning from subroutines
uint16_t SP;            // stack pointer
uint16_t I;             // index register, holds a location in memory, often used for drawing to SCREEN
uint16_t PC;            // program counter, holds next address to be interpreted
uint16_t opcode;        // holds curent instruction to be decoded
u_char DELAY_TIMER;     // will count douwn while its value is greater than 0
u_char SOUND_TIMER;     // will count down, making a noise, while value is greater than 0
u_char SCREEN[64 * 32]; // 64x23 pixel screen, 0x00 -> off and 0xFF -> on


/*========= Memory map =========
    - 0x000 -> 0x1FF    chip8 interpreter( only necicary for character font )
    - 0x050 -> 0x0A0    character fonts location
    - 0x200 -> 0xFFF    program memory and work ram
================================*/

u_char chip8_fontset[80] =
{ 
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


//////////////////////////////////////////////////////////////////////////


void initialise(){
    PC = 0x200;
    SP = 0;
    I = 0;
    DELAY_TIMER = 0;
    SOUND_TIMER = 0; 

    // clear memory
    for(int i = 0x200; i <= 0xFFF; i++){
        RAM[i] = 0x00;
    }

    // load fontset
    for(int i = 0x050; i <= 0x0A0; i++){
        RAM[i] = chip8_fontset[i - 0x050]; 
    }

    // clear stack and registers
    for(int i = 0; i < 16; i++){
        REG[i] = 0;
        STACK[i] = 0;
    }

    // clear the screen
    for(int i = 0; i < 64*32; i++){
        SCREEN[i] = 0;
    }
}


void load_game(std::string game_path){
    std::ifstream file;
    file.open(game_path, std::ios::binary | std::ios::in);

    // reads game rom and writes it to RAM
    if(file.is_open()){

        // find the size of rom file
        file.seekg(0, std::ios::end);
        int size = file.tellg();
        file.seekg(0, std::ios::beg);

        // create a buffer for file input
        std::vector<char> buffer;
        buffer.resize(size);

        // input file contents to buffer
        file.read(buffer.data(), size);
	    file.close();

        // write the buffer into the program portion of RAM
        for(int i = 0; i < size; i++){
            RAM[i + 512] = buffer[i];
	    std::cout << "\n" << RAM[i + 512];
        }


	// couldnt open file
    }else{
	std::cout << "couldnt open file\n";
    }
}
