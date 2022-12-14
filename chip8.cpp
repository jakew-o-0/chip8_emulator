#include<fstream>
#include <iostream>
#include <sys/types.h>
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
    unsigned char SCREEN[64 * 32]; // 64x23 pixel screen, 0x00 -> off and 0xFF -> on

    /*========= Memory map =========
        - 0x000 -> 0x1FF    chip8 interpreter( only necicary for character font )
        - 0x050 -> 0x0A0    character fonts location
        - 0x200 -> 0xFFF    program memory and work ram
    ================================*/


    uint16_t opcode;        // holds curent instruction to be decoded
    unsigned char regX;
    unsigned char regY;
    unsigned char cond_value;


public:
    void initialise();
    void load_game(std::string path);
    void fetch();
    void decode();

private:
    void zero_group();
    void jump_address();
    void call_subroutine();
    void VX_equal_NN();
    void VX_notEqual_NN();
    void VX_equal_VY();
    void set_VX_to_NN();
    void add_NN_to_VX();
    void math_logic_group();
    void VX_notEqual_VY();
    void set_I_to_NNN();
    void jump_to_NNN_plus_V0();
    void VX_to_RAND_AND_NN();
    void draw_to_screen();
    void E_group();
    void F_group();
};





//////////////////////////////////////////////////////////////////////////
///=================== main(public) funcions =========================///
////////////////////////////////////////////////////////////////////////


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


///////////////////////////////////////////////////////////


void Chip8_CPU::load_game(std::string path){
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


void Chip8_CPU::decode() {
    uint16_t op_type = opcode & 0xF000;
    regX = ((opcode & 0x0F00) >> (2 * 8)) & 0xFF;
    regY = ((opcode & 0x00F0) >> 8) & 0xFF;
    cond_value = (opcode & 0x00FF) & 0xFF;

    switch (op_type){

        case 0x0000:
            zero_group();
            break;

        case 0x1000:
            jump_address();
            break;

        case 0x2000:
            call_subroutine();
            break;

        case 0x3000:
            VX_equal_NN();
            break;

        case 0x4000:
            VX_notEqual_NN();
            break;

        case 0x5000:
            VX_equal_VY();
            break;

        case 0x6000:
            VX_equal_NN();
            break;

        case 0x7000:
            add_NN_to_VX();
            break;

        case 0x8000:
            math_logic_group();
            break;

        case 0x9000:
            VX_notEqual_VY();
            break;

        case 0xA000:
            set_I_to_NNN();
            break;

        case 0xB000:
            jump_to_NNN_plus_V0();
            break;

        case 0xC000:
            VX_to_RAND_AND_NN();
            break;

        case 0xD000:
            draw_to_screen();
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


void Chip8_CPU::zero_group(){
    // 0x00E0 clears screen
    if((opcode & 0x00FF) == 0x00E0){/*CLEAR SCREEN*/}

    //0X00EE returns from subroutine
    else if((opcode & 0x00FF) == 0x00EE){
        SP--;
        PC = STACK[SP];
        STACK[SP] = 0x0000;
    }
}


////////////////////////////////////////////////////////////


// 0x1NNN jumps to the address given
// pc set to the given address
void Chip8_CPU::jump_address(){
    PC = opcode & 0x0FFF;
}


///////////////////////////////////////////////////////////


// 0x2NNN calls subroutine at location NNN
// add pc to stack and incement sp
// jump to location NNN by assigning pc NNN
void Chip8_CPU::call_subroutine(){
    STACK[SP] = PC;
    SP++;
    PC = opcode & 0x0FFF;
}


///////////////////////////////////////////////////////////


// 0x3XNN checks if the one byte value NN
// equals the value in register X
// if true, the next instruction is skipped
void Chip8_CPU::VX_equal_NN(){
    if (REG[regX] == cond_value) {
        PC += 2;
    }
}


////////////////////////////////////////////////////////////


// 0x4XNN does the same as 0x3XNN but
// the condition is if X is not equal to NN
void Chip8_CPU::VX_notEqual_NN(){
    if (REG[regX] != cond_value) {
        PC += 2;
    }
}


///////////////////////////////////////////////////////////


// 0x5XY0 compares values of registers X and Y
// if they are equal then the next instruction is skipped
void Chip8_CPU::VX_equal_VY(){
    if (REG[regX] == REG[regY]) {
        PC += 2;
    }
}

///////////////////////////////////////////////////////////


// 0x6XNN sets the value of register X to NN
void Chip8_CPU::set_VX_to_NN(){
    REG[regX] = cond_value;
}


//////////////////////////////////////////////////////////


// 0x7XNN adds byte NN to register VX
// without carry flag
void Chip8_CPU::add_NN_to_VX(){
    REG[regX] += cond_value;
}


/////////////////////////////////////////////////////////


void Chip8_CPU::math_logic_group(){
    u_char op_subType = (opcode & 0x000F) & 0xFF;
    u_char prev_regX = REG[regX];

    switch (op_subType)
    {
    // assigns regX to value of regY
    case 0x00:
        REG[regX] = REG[regY];
        break;



    // assigns regx to value of regy OR regx
    case 0x01:
        REG[regX] |= REG[regY];
        break;



    // assigns regx to value of regy AND regx
    case 0x02:
        REG[regX] &= REG[regY];
        break;



    // assigns regx to value of regy XOR regx
    case 0x03:
        REG[regX] ^= REG[regY];
        break;



    // adds regY to regX with carry flag
    case 0x04:
        prev_regX = REG[regX];
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
        prev_regX = REG[regX];
        REG[regX] -= REG[regY];
        REG[0xF] = 1;

        // if the previous value is smaller than the current,
        // the integer has wrapped and a borrow has taken place
        if (prev_regX < REG[regX]) {
            REG[0xF] = 0;
        }
        break;



    // stores the smallest bit in register F
    // then shifts regx right 1
    case 0x06:
        REG[0xF] = (REG[regX] & 1);
        REG[regX] >>= 1;
        break;



    // sets register x to register y - x
    // with borrow flag
    case 0x07:
        prev_regX = REG[regX];
        REG[regX] = REG[regY] - REG[regX];
        REG[0xF] = 1;

        if (prev_regX < REG[regX]) {
            REG[0xF] = 0;
        }
        break;



    // stores the largest bit in register F
    // then shifts regx left 1
    case 0x0E:
        REG[0xF] = (REG[regX] & (1 << 8));
        REG[regX] <<= 1;
        break;



    default:
        break;
    }
}


////////////////////////////////////////////////////////////////

// 0x9XY0 compares values of registers X and Y
// if they are not equal then the next instruction is skipped
void Chip8_CPU::VX_notEqual_VY(){
    if (REG[regX] != REG[regY]) {
        PC += 2;
    }
}


//////////////////////////////////////////////////////////////////


// 0xANNN asigns the index register to the given address
void Chip8_CPU::set_I_to_NNN(){
    I = opcode & 0x0FFF;
}


//////////////////////////////////////////////////////////////////


// 0xBNNN jumps to location NNN plus register 0
void Chip8_CPU::jump_to_NNN_plus_V0(){
    PC = REG[0] + (opcode & 0x0FFF);
}


//////////////////////////////////////////////////////////////////


// 0xCXNN assigns register x to a random number AND a one byte value given
void Chip8_CPU::VX_to_RAND_AND_NN(){
    REG[regX] = (std::rand() % 255) & cond_value;
}


//////////////////////////////////////////////////////////////////


void draw_to_screen(){

}


////////////////////////////////////////////////////////////////////


void E_group();
void Chip8_CPU::F_group(){
    unsigned char op_subType = (opcode & 0x00FF) & 0xFF;
    switch (op_subType)
    {
    // 0xFX07 assigns register x to the value of the delay timer
    case 0x07:
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
