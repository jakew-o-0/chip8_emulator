#include<fstream>
#include<iostream>
#include<sys/types.h>
#include<vector>
#include<string>

#include "chip8.h"


//////////////////////////////////////////////////////////////////////////
///=================== main(public) funcions =========================///
////////////////////////////////////////////////////////////////////////


void Chip8_CPU::initialise(){   // empties memory and loads the fontset into ram
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


///////////////////////////////////////////////////////////


void Chip8_CPU::load_game(std::string path){    // reads binary file, loads it into ram to be executed
    std::ifstream file;
    file.open(path, std::ios::binary | std::ios::in);

    //cant open file
    if (~(file.is_open())) {

        std::cout << "couldnt open file\n";

    }

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


/////////////////////////////////////////////////////////////


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


///////////////////////////////////////////////////////////


//using the opcode fetched from the fetch function,the binary is decoded
// layed out in two bytes, each nibble is, possibly a diffrent part of an instruction
// the first nibble indicates what type of instruction it is
void Chip8_CPU::decode() {
    uint16_t op_type = opcode & 0xF000;             // isolates most significant nibble, indicates instrucion type
    regX = ((opcode & 0x0F00) >> (2 * 8)) & 0xFF;   // isolates third nibble, often used to point to a register
    regY = ((opcode & 0x00F0) >> 8) & 0xFF;         // isolates second nibble, often used to point to a register 
    cond_value = (opcode & 0x00FF) & 0xFF;          // isolates the two least significant nibbles, often used as a condition or a value

    switch (op_type){
        case 0x0000:
	    
            if((opcode & 0x00f0) == 0x00E0){	// 0x00E0 clears screen
                for(int i =0; i<=32*64; i++){
                    SCREEN[i] = 0x00;
                }
            }

            else if((opcode & 0x00FF) == 0x00EE){	//0X00EE returns from subroutine
                SP--;					//decrement stack pointer
                PC = STACK[SP];				//pc asigned to the retrun location
                STACK[SP] = 0x0000;			//unused stack position is cleared
            }
            break;



        case 0x1000:                // 0x1NNN jumps to the address given 
            PC = opcode & 0x0FFF;   // pc set to the given address
            break;




        case 0x2000:                // 0x2NNN calls subroutine at location NNN
            STACK[SP] = PC;         // add pc to stack and incement sp
            SP++;
            PC = opcode & 0x0FFF;   // jump to location NNN by assigning pc NNN
            break;




        case 0x3000:    // 0x3XNN checks if the one byte value NN equals the value in register X
            if (REG[regX] == cond_value) {  // if true, the next instruction is skipped
                PC += 2;
            }
            break;




        case 0x4000:    // 0x4XNN does the same as 0x3XNN but the condition is if X is not equal to NN
            if (REG[regX] != cond_value) {
                PC += 2;
            }
            break;




        case 0x5000:    // 0x5XY0 compares values of registers X and Y
            if (REG[regX] == REG[regY]) {   // if they are equal then the next instruction is skipped
                PC += 2;
            }
            break;




        case 0x6000:    // 0x6XNN sets the value of register X to NN
            REG[regX] = cond_value;     
            break;




        case 0x7000:    // 0x7XNN adds byte NN to register VX, without carry flag
            REG[regX] += cond_value;
            break;




        case 0x8000:
            math_logic_group();
            break;




        case 0x9000:    // 0x9XY0 compares values of registers X and Y
            if (REG[regX] != REG[regY]) {   // if they are not equal then the next instruction is skipped
                PC += 2;
            }
            break;




        case 0xA000:    // 0xANNN asigns the index register to the given address
            I = opcode & 0x0FFF;
            break;




        case 0xB000:    // 0xBNNN jumps to location NNN plus register 0
            PC = REG[0] + (opcode & 0x0FFF);
            break;




        case 0xC000:   // 0xCXNN assigns register x to a random number AND a one byte value given 
            REG[regX] = (std::rand() % 255) & cond_value;
            break;




        case 0xD000:
            //draw_to_screen();
            break;




        case 0xE000:
            // user input checking
            break;



            
        case 0xF000:
            F_group();
            break;




        default:
            break;
    }
}




///////////////////////////////////////////////////////////
//=================instruction decoding=================///
//////////////////////////////////////////////////////////


void Chip8_CPU::math_logic_group(){
    u_char op_subType = (opcode & 0x000F) & 0xFF;
    u_char prev_regX = REG[regX];

    switch (op_subType){

        case 0x00:  // assigns regX to value of regY
            REG[regX] = REG[regY];
            break;



        case 0x01:  // assigns regx to value of regy OR regx
            REG[regX] |= REG[regY];
            break;


        
        case 0x02:  // assigns regx to value of regy AND regx
            REG[regX] &= REG[regY];
            break;



        case 0x03:  // assigns regx to value of regy XOR regx
            REG[regX] ^= REG[regY];
            break;


        
        case 0x04:  // adds regY to regX with carry flag
            prev_regX = REG[regX];
            REG[regX] += REG[regY];
            REG[0xF] = 0;

            if (prev_regX > REG[regX]) {    // if the previous value is bigger than the current,
                REG[0xF] = 1;               // the integer has wrapped and a carry has taken place
            }
            break;



        case 0x05:  // subtracts regY to regX with borrow flag
            prev_regX = REG[regX];
            REG[regX] -= REG[regY];
            REG[0xF] = 1;

            if (prev_regX < REG[regX]) {    // if the previous value is smaller than the current,
                REG[0xF] = 0;               // the integer has wrapped and a borrow has taken place
            }
            break;



        case 0x06:
            REG[0xF] = (REG[regX] & 1); // stores the smallest bit in register F
            REG[regX] >>= 1;            // then shifts regx right 1
            break;



        case 0x07:  // sets register x to register y - x, with borrow flag
            prev_regX = REG[regX];
            REG[regX] = REG[regY] - REG[regX];
            REG[0xF] = 1;

            if (prev_regX < REG[regX]) {
                REG[0xF] = 0;
            }
            break;


        
        case 0x0E:
            REG[0xF] = (REG[regX] & (1 << 8));  // stores the largest bit in register F
            REG[regX] <<= 1;                    // then shifts regx left 1
            break;



        default:
            break;
    }
}


//////////////////////////////////////////////////////////////////


void draw_to_screen(){

}


////////////////////////////////////////////////////////////////////


void E_group();
void Chip8_CPU::F_group(){
    u_char op_subType = (opcode & 0x00FF) & 0xFF;

    switch (op_subType){

    
        case 0x07:  // 0xFX07 assigns register x to the value of the delay timer
            REG[regX] = DELAY_TIMER;
            break;

        case 0x0A:
            //waits for key input
            break;
            
        case 0x15:
            DELAY_TIMER = REG[regX];

        case 0x18:
            SOUND_TIMER = REG[regX];
            break;

        case 0x1E:
            I += REG[regX];
            break;

        case 0x29:
            //something silly
            break;

        case 0x33:
        break; 

        default:
            break;
    }
}


////////////////////////////////////////////////////////////////////////
