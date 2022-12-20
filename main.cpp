#include "chip8.h"
#include "user_interface.h"
#include <curses.h>
#include <iterator>
#include <ncurses.h>


int main(int argc, char** argv){
    usr_interface screen;
    screen.init();
    
    unsigned char input;
    while (1) {
        input = screen.get_input(); 
        if(input == 0xF0){
            endwin();
            return 0;
        }
    }
}
