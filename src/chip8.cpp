#include <cstdio>
#include <cstdlib>
#include<fstream>
#include<iostream>
#include<sys/types.h>
#include<vector>
#include<string>
#include <unistd.h>

#include "/home/jake/code/c_cpp/chip8_emulator/headers/chip8.h"





//////////////////////////////////////////////////////////////////////////
///=================== main(public) funcions =========================///
////////////////////////////////////////////////////////////////////////



// empties memory and loads the fontset into ram

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

    
    for(int i = 0x200; i <= 0xFFF; i++){    // clear memory
        RAM[i] = 0x00;
    }

    
    for(int i = 0x050; i <= 0x0A0; i++){    // load fontset
        RAM[i] = chip8_fontset[i - 0x050]; 
    }

    
    for(int i = 0; i < 16; i++){    // clear stack and registers
        REG[i] = 0;
        STACK[i] = 0;
    }

    for(int i = 0; i < 32; i++)
    {
          for(int j = 0; j < 64; j++)
          {
                  SCREEN[i][j] = 0x0;
          }
    }
}






///////////////////////////////////////////////////////////






/*
 * reads the ch8 game rom and loads it into ram
 * params:
 *      path -> path to rom file
 */

void Chip8_CPU::load_game(std::string path)
{
    std::ifstream file;
    file.open(path.c_str(), std::ios::binary | std::ios::in);

    
    if (!file.is_open()) {    //cant open file

        std::cout << "couldnt open file\n";
        exit(-1);

    }

    // reads game rom and writes it to RAM
    
    file.seekg(1, std::ios::end);   // find the size of rom 
    int size = file.tellg();
    file.seekg(0, std::ios::beg);

    
    std::vector<char> buffer;   // create a buffer for file input
    buffer.resize(size);

    
    file.read(buffer.data(), size); // input file contents to buffer
    file.close();

    
    for (int i = 0; i < size; i++) {    // write the buffer into the program portion of RAM
        RAM[i + 512] = buffer[i];
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

void Chip8_CPU::fetch() 
{
      opcode = (RAM[PC] <<  8) | RAM[PC + 1] ;
      PC += 2;

      if(DELAY_TIMER > 0)
      {
            DELAY_TIMER--;
      }
      
      if(SOUND_TIMER > 0)
      {
            SOUND_TIMER--;
      }
}







///////////////////////////////////////////////////////////







//using the opcode fetched from the fetch function,the binary is decoded
// layed out in two bytes, each nibble is, possibly a diffrent part of an instruction
// the first nibble indicates what type of instruction it is
//
void Chip8_CPU::decode() 
{
      uint16_t op_type = opcode & 0xF000;      // isolates most significant nibble, indicates instrucion type
      regX = (opcode & 0x0F00) >>  8;          // isolates third nibble, often used to point to a register
      regY = (opcode & 0x00F0) >> 4;           // isolates second nibble, often used to point to a register 
      cond_value = opcode & 0x00FF;            // isolates the two least significant nibbles, often used as a condition or a value
      op_subType = opcode & 0x000F;            // last nibble



      switch (op_type){
            case 0x0000:
            {
                  if(cond_value == 0xE0){	// 0x00E0 clears screen
                        screen_cond_flag = 0x0;

                        for(int i = 0; i < 32; i++)
                        {
                              for(int j = 0; j < 64; j++)
                              {
                                    SCREEN[i][j] = 0x0;
                              }
                        }
                  }

                  else if(cond_value == 0xEE){	//0X00EE returns from subroutine
                        SP--;					      //decrement stack pointer
                        PC = STACK[SP];				//pc asigned to the retrun location
                        STACK[SP] = 0x0000;			//unused stack position is cleared
                  }
                  break;
            }



            case 0x1000:                  // 0x1NNN jumps to the address given 
            {
                  PC = opcode & 0x0FFF;   // pc set to the given address
                  break;
            }




            case 0x2000:                  // 0x2NNN calls subroutine at location NNN
            {
                  STACK[SP] = PC;         // add pc to stack and incement sp
                  SP++;
                  PC = opcode & 0x0FFF;   // jump to location NNN by assigning pc NNN
                  break;
            }




            case 0x3000:    // 0x3XNN checks if the one byte value NN equals the value in register X
            {
                  if (REG[regX] == cond_value) {  // if true, the next instruction is skipped
                        PC += 2;
                  }
                  break;
            }




            case 0x4000:    // 0x4XNN does the same as 0x3XNN but the condition is if X is not equal to NN
            {
                  if (REG[regX] != cond_value) {
                        PC += 2;
                  }
                  break;
            }




            case 0x5000:    // 0x5XY0 compares values of registers X and Y
            {
                  if (REG[regX] == REG[regY]) {   // if they are equal then the next instruction is skipped
                        PC += 2;
                  }
                  break;
            }




            case 0x6000:    // 0x6XNN sets the value of register X to NN
            {
                  REG[regX] = cond_value;     
                  break;
            }




            case 0x7000:    // 0x7XNN adds byte NN to register VX, without carry flag
            {
                  REG[regX] += cond_value;
                  break;
            }




            case 0x8000:
            {
                  math_logic_group();
                  break;
            }




            case 0x9000:    // 0x9XY0 compares values of registers X and Y
            {
                  if (REG[regX] != REG[regY]) {   // if they are not equal then the next instruction is skipped
                        PC += 2;
                  }
                  break;
            }




            case 0xA000:    // 0xANNN asigns the index register to the given address
            {
                  I = opcode & 0x0FFF;
                  break;
            }




            case 0xB000:    // 0xBNNN jumps to location NNN plus register 0
            {
                  PC = REG[0] + (opcode & 0x0FFF);
                  break;
            }




            case 0xC000:   // 0xCXNN assigns register x to a random number AND a one byte value given 
            {
                  REG[regX] = (std::rand() % 255) & cond_value;
                  break;
            }




            case 0xD000:
            {
                  draw_to_screen();
                  break;
            }




            case 0xE000:
            {
                  E_group();
                  break;
            }




            case 0xF000:
            {
                  F_group();
                  break;
            }




            default:
                  break;
      }
}




///////////////////////////////////////////////////////////
//=================instruction decoding=================///
//////////////////////////////////////////////////////////


void Chip8_CPU::math_logic_group()
{
      unsigned char prev_regX = REG[regX];    //used for borrow checking 

      switch (op_subType)
      {

            case 0x00:  // assigns regX to value of regY
            {
                  REG[regX] = REG[regY];
                  break;
            }




            case 0x01:  // assigns regx to value of regy OR regx
            {
                  REG[regX] |= REG[regY];
                  REG[0xF] = 0;
                  break;
            }

            

        
            case 0x02:  // assigns regx to value of regy AND regx
            {
                  REG[regX] &= REG[regY];
                  REG[0xF] = 0;
                  break;
            }




            case 0x03:  // assigns regx to value of regy XOR regx
            {
                  REG[regX] ^= REG[regY];
                  REG[0xF] = 0;
                  break;
            }



        
            case 0x04:  // adds regY to regX with carry flag
            {
                  prev_regX = REG[regX];
                  REG[regX] += REG[regY];
                  REG[0xF] = 0;

                  if (prev_regX > REG[regX]) {    // if the previous value is bigger than the current,
                        REG[0xF] = 1;               // the integer has wrapped and a carry has taken place
                  }

                  break;
            }




            case 0x05:  // subtracts regY to regX with borrow flag
            {
                  prev_regX = REG[regX];
                  REG[regX] -= REG[regY];
                  REG[0xF] = 1;

                  if (prev_regX < REG[regX]) 
                  {                                 // if the previous value is smaller than the current,
                        REG[0xF] = 0;               // the integer has wrapped and a borrow has taken place
                  }
                  break;
            }




            case 0x06:
            {
                  REG[regX] = REG[regY];
                  unsigned char small_bit = (REG[regX] & 0x01); // stores the smallest bit in register F
                  REG[regX] >>= 1;            // then shifts regx right 1
                  REG[0xF] = small_bit;
                  break;
            }




            case 0x07:  // sets register x to register y - x, with borrow flag
            {
                  prev_regX = REG[regX];
                  REG[regX] = REG[regY] - REG[regX];
                  REG[0xF] = 0;

                  if (prev_regX < REG[regX]) 
                  {
                        REG[0xF] = 1;
                  }
                  break;
            }



        
            case 0x0E:
            {
                  REG[regX] = REG[regY];
                  unsigned char significant_bit = (REG[regX] & (1 << 7)) >> 7;  // stores the largest bit in register F
                  REG[regX] <<= 1;                    // then shifts regx left 1
                  REG[0xF] = significant_bit;
                  break;
            }



            default:
                  break;
      }
}


//////////////////////////////////////////////////////////////////


void Chip8_CPU::E_group()
{
      switch (cond_value) 
      {

            case 0x9E:                        // if the curent input key is equal
            {
                  if(key == REG[regX])
                  {                           // to the value in regX then 
                        PC += 2;              // the next instrucion is skipped
                  }
                  break;
            }

        

            case 0xA1:  // same as 9E but not equal to
            {
                  if(key != REG[regX])
                  {
                       PC += 2;
                  }
                  break;
            }


        
            case 0x07:  // assigns regX to the value of DELAY_TIMER
            {
                  REG[regX] = DELAY_TIMER;
                  break;
            }



            default: break;
      }
}


////////////////////////////////////////////////////////////////////

    
void Chip8_CPU::F_group()
{
      switch (cond_value)
      {

    
            case 0x07:  // 0xFX07 assigns register x to the value of the delay timer
            {
                  REG[regX] = DELAY_TIMER;
                  break;
            }




            case 0x0A:  //waits for key input
            {
                  if(key != REG[regX])
                  {
                        PC -= 2;
                  }
                  break;
            }



            
            case 0x15:  // sets delay to value in regX
            {
                  DELAY_TIMER = REG[regX];
            }




            case 0x18:  // sets sound to value in regX
            {
                  SOUND_TIMER = REG[regX];
                  break;
            }




            case 0x1E: // increments I by the value of regX
            {
                  I += REG[regX];
                  break;
            }




            case 0x29: // point I to the character fontset coresponding to the character in regX
                  switch (REG[regX]) 
                  {
                        case 0x00:  I = 80; break;  // 0
                        case 0x01:  I = 85; break;  // 1
                        case 0x02:  I = 90; break;  // 2
                        case 0x03:  I = 95; break;  // 3
                        case 0x04:  I = 100; break; // 4
                        case 0x05:  I = 105; break; // 5
                        case 0x06:  I = 110; break; // 6
                        case 0x07:  I = 115; break; // 7
                        case 0x08:  I = 120; break; // 8
                        case 0x09:  I = 125; break; // 9
                        case 0x0A:  I = 130; break; // A
                        case 0x0B:  I = 135; break; // B
                        case 0x0C:  I = 140; break; // C
                        case 0x0D:  I = 145; break; // D
                        case 0x0E:  I = 150; break; // E
                        case 0x0F:  I = 155; break; // F

                        default: break;
                  }
                  break;




              case 0x33:
                        // BCI CONVERSION
                    break; 

              case 0x55:
              {
                    
                    for(int i = 0; i <= regX; i++)
                    {
                          RAM[I + i] = REG[i];
                    }
                    break;
              }


              case 0x65:
              {
                    
                    for(int i = 0; i <= regX; i++)
                    {
                          REG[i] = RAM[I + i];
                    }
                    break;
              }



              default: break;
      }
}


///////////////////////////////////////////////////////////////////////


void Chip8_CPU::draw_to_screen()
{                                     // processes data in the instrucion to be easily used by the ui
      screen_cond_flag = 0xF;
      int height = op_subType;
      int pos_x = REG[regX];
      int pos_y = REG[regY];

      //if(REG[regX] > 64 || REG[regY] > 32)
      //{
            //pos_x = REG[regX] % 32;
            //pos_y = REG[regY] % 64;
      //}



      for(int i = 0; i < height; i++)
      {
            int y_offset = 0;

            for(int j = 7; j >= 0; j--)
            {
                  if(pos_y + i > 32 || pos_x + y_offset > 64)
                  {
                       break; 
                  }


                  // masks the j'th bit from the location in memory
                  // that I points to and shifts it to the least
                  // significant bit
                  unsigned char bit = (RAM[I + i] & (1 << j)) >> j;
                  SCREEN[pos_y + i][pos_x + y_offset] ^= bit;       
                  
                  if(SCREEN[pos_y + i][pos_x + y_offset] == 0)
                  {
                        REG[0xF] = 1;
                  }
                  else
                  {
                        REG[0xF] = 0;
                  }

                  y_offset++;
            }
      }
}



///////////////////////////////////////////////////////////
//===================== debugging ======================//
/////////////////////////////////////////////////////////

void Chip8_CPU::debug(){

    for(int i = 0; i < 16; i++){
        printf("Register %d: %x\n", i, REG[i]);
 
   }
   

   std::cout << "\n\nstack:\n";


    for(int i = 0; i < 16; i++){
        printf("%d: %x\n", i, STACK[i]);
   }

   printf("\nstack pointer: %x\n", SP);
   printf("I register: %x\n", I);
   printf("program counter: %d, %x\n", PC, PC);

   printf("delay timer: %d\n", DELAY_TIMER);
   printf("sound timer: %d\n", SOUND_TIMER);

   printf("\nopcode: %x:", opcode);
   printf("\nregX: %x", regX);
   printf("\nregY: %x", regY);
   printf("\ncond_value: %x", cond_value);
   printf("\nop_subtype: %x", op_subType);
   printf("\n\n\n");
   printf("desplayed bit: %x", RAM[I]);


   std::cin.get();
}
