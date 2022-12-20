#ifndef _USER_INTERFACE_
#define _USER_INTERFACE_

#include <ncurses.h>

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
};

#endif
