#include <locale>
#include <ncurses.h>

#include "user_interface.h"

void usr_interface::init(){
	setlocale(LC_ALL, "");  // ensures locale is used to avoid encoding errors with unicode
	initscr();              // initialises screen
    noecho();
	curs_set(false);        // hides curser
    cbreak();
    nodelay(stdscr, true);

	getmaxyx(stdscr, max_y, max_x); // get max size of the screen 

	int winmax_y = (max_y/2) - 18;  // centers the windows and offsets them
	int winmax_x = (max_x/2) - 64;
	int info_max_y =(winmax_y) + 35; 
	int info_max_x =(max_x/2) - 32; 

    win = newwin(33,130, winmax_y,winmax_x);    // create windows 
	info_win = newwin(6,65, info_max_y,info_max_x);
    refresh();

	box(win, 0,0);          // add boxes to the windows
	box(info_win, 0,0);
	wrefresh(win);
	wrefresh(info_win);


    draw_info_window(); // draw unchanging data to info window
	wrefresh(info_win);
}


//////////////////////////////////////////////////////////


void usr_interface::draw_info_window(){
	char* info[6] = {"esc to quit", "controlls:", "1,2,3,4", "q,w,e,r", "a,s,d,f", "z,x,c,v"};

	mvwprintw(info_win, 1,12, info[0]);
	mvwprintw(info_win, 1,35, info[1]);
	int j = 1;
	for(int i = 2; i<=5;i++){	
		mvwprintw(info_win, j,47, info[i]);
		j++;
	}
}


///////////////////////////////////////////////////////////


char usr_interface::get_input(){
    char c = getch();
    if(c != ERR){ 
        switch (c) {    // converts the keyboard into the chip8 keypad
            case '1': return 0x01;
            case '2': return 0x02;
            case '3': return 0x03;
            case '4': return 0x0C;
            case 'q': return 0x04;
            case 'w': return 0x05;
            case 'e': return 0x06;
            case 'r': return 0x0D;
            case 'a': return 0x07;
            case 's': return 0x08;
            case 'd': return 0x09;
            case 'f': return 0x0E;
            case 'z': return 0x0A;
            case 'x': return 0x00;
            case 'c': return 0x0B;
            case 'v': return 0x0F;

            case 27: return 0xF0;   // esc is pressed, end program

            default: return 0xFF;
        }
    }else{return 0xFF;}
}
//////////////////////////////////////////////////////////
