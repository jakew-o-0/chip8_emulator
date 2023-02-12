#include "/home/jake/code/c_cpp/chip8_emulator/headers/chip8.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <ostream>


int main(int argc, char** argv)
{
      Chip8_CPU chip8;
      chip8.initialise();
      chip8.load_game("test_opcode.ch8");





      ///initialise video
      ///
      if(SDL_Init(SDL_INIT_VIDEO) < 0)
      {
            std::cout << "video init has failed" << std::endl;
            return -1; 
      }
      



      ///create a window
      ///
      SDL_Window* window = SDL_CreateWindow("Chip8 Emulator", 
                                SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED,
                                660,
                                340,
                                SDL_WINDOW_SHOWN);
      if(window == NULL)
      {
            std::cout << "window has failed to create" << std::endl;
            return -1;
      }



      
      ///create a renterer for the window
      ///
      SDL_Renderer* window_renderer = SDL_CreateRenderer(window,
                                                         -1,
                                                         SDL_RENDERER_ACCELERATED);
      if(window_renderer == NULL)
      {
            std::cout << "renderer failed to create" << std::endl;
            return -1;
      }


      SDL_Event event_stack;
      


      while (1) {     // main loop

            ////////////////////////////////////
            /// fetch, decode, execute cycle

            chip8.fetch();
            chip8.decode();


            while (SDL_PollEvent(&event_stack)) 
            {
                  switch (event_stack.type) 
                  {
                        case SDL_QUIT:
                              SDL_DestroyRenderer(window_renderer);
                              SDL_DestroyWindow(window);
                              exit(0);

                        default: break;
                  }
            }


            chip8.debug();
      }
}
