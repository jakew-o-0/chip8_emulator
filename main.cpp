#include "chip8.h"
#include "user_interface.h"
#include <curses.h>
#include <iterator>
#include <ncurses.h>


int main(int argc, char** argv){
    usr_interface screen;
    screen.init();
    unsigned char input;

    Chip8_CPU chip8;
    chip8.initialise();

    while (1) {     // main loop

        ///////////////////////////////////
        /// poll input

        input = screen.get_input(); 

        if(input == 0xF0){  // escape has been pressed, game will end
            endwin();
            return 0;
        }

        chip8.key = input;  // input is stored in chip8 to be used in its logic

        ////////////////////////////////////
        /// fetch, decode, execute cycle
        
        //chip8.fetch();
        //chip8.decode();

        ////////////////////////////////////
    }
}
