#include "chip8.h"
//#include "user_interface.h"
//#include <curses.h>
//#include <iterator>
//#include <ncurses.h>


int main(int argc, char** argv){
    //usr_interface screen;
    //screen.init();
    unsigned char input;

    Chip8_CPU chip8;
    chip8.initialise();
    chip8.load_game("test_opcode.ch8");

    while (1) {     // main loop

        ///////////////////////////////////
        /// poll input

        //input = screen.get_input(); 

        //if(input == 0xF0){  // escape has been pressed, game will end
         //   endwin();
         //   return 0;
        //}

        //chip8.key = input;  // input is stored in chip8 to be used in its logic

        ////////////////////////////////////
        /// fetch, decode, execute cycle
        
        chip8.fetch();
        chip8.decode();
        chip8.debug();

        ////////////////////////////////////
        /// update screen

        //screen.draw_screen(chip8.x_pos, chip8.y_pos, chip8.sprite, chip8.screen_cond);
    }
}
