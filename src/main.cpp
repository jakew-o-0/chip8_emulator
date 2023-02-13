#include "/home/jake/code/c_cpp/chip8_emulator/headers/chip8.h"
#include <SDL2/SDL_rect.h>
#include <algorithm>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_video.h>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <ostream>


int main(int argc, char** argv)
{
      Chip8_CPU chip8;
      chip8.initialise();
      chip8.load_game("/home/jake/code/c_cpp/chip8_emulator/resorces/test_opcode.ch8");





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
      


      while (1) // main loop
      {           
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



            ////////////////////////////////////
            /// fetch, decode, execute cycle

            chip8.fetch();
            chip8.decode();


            if(chip8.screen_cond_flag == 0xF)
            {
                  int pos_x = 0;
                  int pos_y = 0;
                  std::vector<SDL_Rect> pixels;

                  for(int i = 0; i < 64*32; i++)
                  {
                        if(i % 64 == 0)
                        {
                              pos_y++;
                              pos_x = 0;
                        }

                        if(chip8.SCREEN[i] == 0x1)
                        {
                              SDL_Rect tmp;
                              tmp.h = 10;
                              tmp.w = 10;
                              tmp.x = pos_x;
                              tmp.y = pos_y;

                              pixels.push_back(tmp);
                        }

                        pos_x++;
                  }

                  SDL_Rect pixel_arr[pixels.size()];
                  std::copy(pixels.begin(), pixels.end(), pixel_arr);

                  SDL_SetRenderDrawColor(window_renderer, 225, 225, 225, 225);
                  SDL_RenderFillRects(window_renderer, pixel_arr, pixels.size());
                  SDL_RenderDrawRects(window_renderer, pixel_arr, pixels.size());
                  SDL_SetRenderDrawColor(window_renderer, 0, 0, 0, 225);
            }
            else if(chip8.screen_cond_flag == 0x0)
            {
                  SDL_RenderClear(window_renderer);
            }


            chip8.debug();
      }
}




