#include<fstream>
#include <iostream>
#include<vector>
#include<string>


class Chip8_CPU
{
    unsigned char RAM[4096];       // 4KB memory
    unsigned char REG[16];         // general purpose registers 0-F, F is used as a carry flag
    uint16_t STACK[16];     // calling and returning from subroutines
    uint16_t SP;            // stack pointer
    uint16_t I;             // index register, holds a location in memory, often used for drawing to SCREEN
    uint16_t PC;            // program counter, holds next address to be interpreted
    uint16_t opcode;        // holds curent instruction to be decoded
    unsigned char DELAY_TIMER;     // will count douwn while its value is greater than 0
    unsigned char SOUND_TIMER;     // will count down, making a noise, while value is greater than 0
    unsigned char SCREEN[64 * 32]; // 64x23 pixel screen, 0x00 -> off and 0xFF -> on


    /*========= Memory map =========
        - 0x000 -> 0x1FF    chip8 interpreter( only necicary for character font )
        - 0x050 -> 0x0A0    character fonts location
        - 0x200 -> 0xFFF    program memory and work ram
    ================================*/

    
    void initialise();
    void load_game(std::string path);
    void fetch();
    void decode();
};





//////////////////////////////////////////////////////////////////////////


void Chip8_CPU::initialise(){
    PC = 0x0200;
    SP = 0;
    I = 0;
    DELAY_TIMER = 0;
    SOUND_TIMER = 0;

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


void Chip8_CPU::load_game(std::string path){
    std::ifstream file;
    file.open(game_path, std::ios::binary | std::ios::in);

    //cant open file
    if (~(file.is_open())) {

        std::cout << "couldnt open file\n";

    }
    else {
        // reads game rom and writes it to RAM
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
        for (int i = 0; i < size; i++) {
            RAM[i + 512] = buffer[i];
            std::cout << "\n" << RAM[i + 512];
        }
    }
}


// assigns opcode to where the pc is pointing to in ram
// on the first loop PC will equal 0x200
// instructions are 2 bytes and in 1 byte addresses in ram
// the first half must be shifted to the left twice going from
// a 0xNN value of one byte to 0xNN00 two byte value
// this can be binary OR-ed with the second half 
// in the next position in memory
// PC is then incremented by two
void Chip8_CPU::fetch() {
    opcode = (RAM[PC] << (2 * 8)) | RAM[PC + 1] ;
    PC += 2;
}


void Chip8_CPU::decode() {
    uint16_t op_type = opcode & 0xF000;
    unsigned char regX = ((opcode & 0x0F00) >> (2 * 8)) & 0xFF;
    unsigned char regY = ((opcode & 0x00F0) >> 8) & 0xFF;
    unsigned char cond_value = (opcode & 0x00FF) & 0xFF;

    switch (op_type){

        case 0x0000:
            // 0x00E0 clears screen
            if(opcode & 0x00FF == 0x00E0){/*CLEAR SCREEN*/}
            //0X00EE returns from subroutine
            else if(opcode & 0x00FF == 0x00EE){
                SP--;
                PC = STACK[SP];
                STACK[SP] = 0x0000;
            }
            break;


        // 0x1NNN jumps to the address given
        // pc set to the given address
        case 0x1000:
            PC = opcode & 0x0FFF;
            break;


        // 0x2NNN calls subroutine at location NNN
        // add pc to stack and incement sp
        // jump to location NNN by assigning pc NNN
        case 0x2000:
            STACK[SP] = PC;
            SP++;
            PC = opcode & 0x0FFF;
            break;


        // 0x3XNN checks if the one byte value NN
        // equals the value in register X
        // if true, the next instruction is skipped
        case 0x3000:
            if (REG[regX] == cond_value) {
                PC += 2;
            }
            break;


        // 0x4XNN does the same as 0x3XNN but
        // the condition is if X is not equal to NN
        case 0x4000:
            if (REG[regX] != cond_value) {
                PC += 2;
            }
            break;


        // 0x5XY0 compares values of registers X and Y
        // if they are equal then the next instruction is skipped
        case 0x5000:
            if (REG[regX] == REG[regY]) {
                PC += 2;
            }
            break;


        // 0x6XNN sets the value of register X to NN
        case 0x6000:
            REG[regX] = cond_value;
            break;


        // 0x7XNN adds value NN to whats alreadsy in
        // register X and assigns that to register X
        case 0x7000:
            REG[regX] += cond_value;
            break;


        // group of mathmatical and logical commands
        case 0x8000:
            unsigned char op_subType = (opcode & 0x000F) & 0xFF;
            switch (op_subType)
            {
            // assigns regX to value of regY
            case 0x00:
                REG[regX] = REG[regY];

            // assigns regx to value of regy OR regx
            case 0x01:
                REG[regX] |= REG[regY];

            // assigns regx to value of regy AND regx
            case 0x02:
                REG[regX] &= REG[regY];

            // assigns regx to value of regy XOR regx
            case 0x03:
                REG[regX] ^= REG[regY];

            // adds regY to regX with carry flag
            case 0x04:
                unsigned char prev_regX = REG[regX];
                REG[regX] += REG[regY];
                REG[0xF] = 0;

                // if the previous value is bigger than the current,
                // the integer has wrapped and a carry has taken place
                if (prev_regX > REG[regX]) {
                    REG[0xF] = 1;
                }
                break;

            // subtracts regY to regX with borrow flag
            case 0x05:
                unsigned char prev_regX = REG[regX];
                REG[regX] -= REG[regY];
                REG[0xF] = 1;

                // if the previous value is smaller than the current,
                // the integer has wrapped and a borrow has taken place
                if (prev_regX < REG[regX]) {
                    REG[0xF] = 0;
                }
                break;

            default:
                break;
            }
            break;


        // 0x9XY0 compares values of registers X and Y
        // if they are not equal then the next instruction is skipped
        case 0x9000:
            if (REG[regX] != REG[regY]) {
                PC += 2;
            }
            break;


        // 0xANNN asigns the index register to the given address
        case 0xA000:
            I = opcode & 0x0FFF;
            break;


        case 0xD000:
            //draw to screen
            // TODO sort out GUI

        default:
            break;
    }
}
