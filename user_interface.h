#ifndef _USER_INTERFACE_
#define _USER_INTERFACE_

#include <ncurses.h>
#include <string>
#include <vector>

class usr_interface{
    private:
        int max_x;
        int max_y;

        WINDOW* info_win;
        WINDOW* win;

    public:
        void init();
        void draw_info_window();
        char get_input();
        void draw_screen(unsigned char x_pos,unsigned char y_pos,std::vector<std::string> sprite, unsigned char screen_cond);
};

#endif
