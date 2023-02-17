#include "/home/jake/code/c_cpp/chip8_emulator/headers/chip8.h"
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_timer.h>
#include <cstdio>
#include <vector>
#include <unistd.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_video.h>
#include <cstddef>
#include <cstdlib>
#include <ostream>


int main(int argc, char** argv)
{
      Chip8_CPU chip8; 
      chip8.initialise(); 
      chip8.load_game("/home/jake/code/c_cpp/chip8_emulator/resorces/chip8-test-suite.ch8");





      ///initialise video
      ///
      if(SDL_Init(SDL_INIT_EVERYTHING) < 0)
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

            int starting_tick = SDL_GetTicks();


            while (SDL_PollEvent(&event_stack)) 
            {
                  switch (event_stack.type) 
                  {
                        case SDL_QUIT:
                              SDL_DestroyRenderer(window_renderer);
                              SDL_DestroyWindow(window);
                              SDL_Quit();
                              exit(0);

                        case SDL_KEYUP:
                              switch (event_stack.key.keysym.sym) 
                              {
                                    case SDLK_1:
                                          chip8.key = 0x01; break;
                                    case SDLK_2:
                                          chip8.key = 0x02; break;
                                    case SDLK_3:
                                          chip8.key = 0x03; break;
                                    case SDLK_4:
                                          chip8.key = 0x0C; break;
                                    case SDLK_q:
                                          chip8.key = 0x04; break;
                                    case SDLK_w:
                                          chip8.key = 0x05; break;
                                    case SDLK_e:
                                          chip8.key = 0x06; break;
                                    case SDLK_r:
                                          chip8.key = 0x0D; break;
                                    case SDLK_a:
                                          chip8.key = 0x07; break;
                                    case SDLK_s:
                                          chip8.key = 0x08; break;
                                    case SDLK_d:
                                          chip8.key = 0x09; break;
                                    case SDLK_f:
                                          chip8.key = 0x0E; break;
                                    case SDLK_z:
                                          chip8.key = 0x0A; break;
                                    case SDLK_x:
                                          chip8.key = 0x00; break;
                                    case SDLK_c:
                                          chip8.key = 0x0B; break;
                                    case SDLK_v:
                                          chip8.key = 0x0F; break;
                                    default: break;
                              }

                        default: break;
                  }
            }







            ////////////////////////////////////
            /// fetch, decode, execute cycle

            chip8.fetch();
            chip8.decode();
            //chip8.debug();
            





            ///////////////////////////////////
            /// draw screen if the draw
            /// instruction has been called 

            if(chip8.screen_cond_flag == 0xF)
            {
                  chip8.screen_cond_flag = 0x1;            // unset the draw flag so the screen is only rendered when the draw instruction is used
                  SDL_RenderClear(window_renderer);        // clear the screen
                  std::vector<SDL_Rect> pixels;            // list of on pixels to be drawn

                  for(int i = 0; i < 32; i++)              // itterates through the SCREEN abstraction, where its a one a pixel will be added to the
                  {                                        // list with the position of i and j, multiplied by 10 as the x and y co-ordernates, +10 is to add padding around the screen
                        for(int j = 0; j < 64; j++)
                        {
                              if(chip8.SCREEN[i][j] == 1)
                              {
                                    SDL_Rect tmp;
                                    tmp.h = 10;
                                    tmp.w = 10;
                                    tmp.x = j*10+10;
                                    tmp.y = i*10+10;

                                    pixels.push_back(tmp);
                              }
                        }
                  }

                  SDL_Rect pixel_arr[pixels.size()];
                  std::copy(pixels.begin(), pixels.end(), pixel_arr);

                  SDL_SetRenderDrawColor(window_renderer, 225, 225, 225, 225);
                  SDL_RenderFillRects(window_renderer, pixel_arr, pixels.size());
                  SDL_RenderDrawRects(window_renderer, pixel_arr, pixels.size());

                  SDL_SetRenderDrawColor(window_renderer, 0, 0, 0, 225);
                  SDL_RenderPresent(window_renderer);


            }
            else if(chip8.screen_cond_flag == 0x0)      // the clear screen instruction has been called
            {
                  chip8.screen_cond_flag = 0x1;
                  SDL_RenderClear(window_renderer);
                  SDL_RenderPresent(window_renderer);
            }





            if( (1000 / 60) > SDL_GetTicks() - starting_tick )
            {
                  SDL_Delay( 1000 / 150 - ( SDL_GetTicks() - starting_tick));
            }
      }
}
