#include <SDL.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <math.h>
#include "main.h"

#define assert(value) do { if (!(value)) *(i32 *)0 = 0; } while (0)
#define array_count(array) sizeof(array) / sizeof(array[0])
#define loop_for(index_name, count) for (i32 index_name = 0; index_name < count; ++index_name)
#define for_each(it_name, array) for (i32 it_name = 0; it_name < array_count(array); ++it_name)
#define for_each_it(it_name, array, it_type) for (it_type it_name = &array[0]; it_name != (array + array_count(array)); ++it_name)

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 800
global SDL_Renderer *global_renderer;

i32 ranged_rand(i32 lo, i32 hi)
{
    assert(hi > lo);
    i32 result = rand() % (hi - lo) + lo;
    return result;
}

v2i v2i_zero()
{
    v2i result = {0, 0};
    return result;
}

v2i make_v2i(i32 x, i32 y)
{
    return {x, y};
}

bool operator==(v2i a, v2i b)
{
    return a.x == b.x && a.y == b.y;
}

v2i operator+(v2i a, v2i b)
{
    return {a.x + b.x, a.y + b.y};
}

i32 clamp(i32 value, i32 min, i32 max)
{
    assert(max >= min);
    i32 result = value;
    if (value < min)
    {
        result = min;
    }
    else if (value > max)
    {
        result = max;
    }
    return result;
}

#include "tile_map.h"

#define TILE_SIZE 10
void render(Tile_Map *tile_map)
{
    SDL_SetRenderDrawColor(global_renderer, 0x10, 0x10, 0x10, 0xff);
    {
        SDL_Rect fill_rect = {0, WINDOW_HEIGHT - tile_map->y_count*TILE_SIZE,
                              tile_map->x_count*TILE_SIZE, tile_map->y_count*TILE_SIZE};
        SDL_RenderFillRect(global_renderer, &fill_rect);
    }

    SDL_SetRenderDrawColor(global_renderer, 0, 0xff, 0, 0xff);
    loop_for(x, tile_map->x_count)
    {
        loop_for(y, tile_map->y_count)
        {
            if (tile_map->get_tile({x, y}) != 0)
            {
                SDL_Rect tile_rect = {x*TILE_SIZE, WINDOW_HEIGHT - (y+1)*TILE_SIZE, TILE_SIZE, TILE_SIZE};
                SDL_RenderFillRect(global_renderer, &tile_rect);
            }
        }
    }
}

#include <windows.h>
int CALLBACK
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR cmdLine, int cmdShow)
{
    srand((u32)time(0));
    
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window *Window = SDL_CreateWindow("Tester Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    global_renderer = SDL_CreateRenderer(Window, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
    
    Tile_Map tile_map = {};
    tile_map.init(80, 80);
    
    b32 GameRunning = true;
    while (GameRunning)
    {
        u32 tick_before_proc = SDL_GetTicks();
            
        SDL_Event Event;
        while (SDL_PollEvent(&Event))
        {
            switch (Event.type)
            {
                case SDL_QUIT:
                {
                    GameRunning = false;
                } break;
                
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                {
                    SDL_Keycode KeyCode = Event.key.keysym.sym;
                    b32 KeyDown = (Event.key.state == SDL_PRESSED);
                    b32 KeyWasDown = (Event.key.repeat != false);
                    
                    if (KeyWasDown != KeyDown)
                    {
                        if (KeyDown)
                        {
                            if (KeyCode == SDLK_ESCAPE)
                            {
                                GameRunning = false;
                            }
                            
                            if (KeyCode == SDLK_r)
                            {
                                tile_map.init(80, 80);
                            }
                        }
                    }
                } break;
            }
        }
        
        SDL_SetRenderDrawColor(global_renderer, 0, 0, 0, 0xFF);
        SDL_RenderClear(global_renderer);
        render(&tile_map);
        
        u32 tick_after_proc = SDL_GetTicks();
        
        //report ms per frame
        {
            char buffer[50] = {};
            u32 ticks_in_between = tick_after_proc - tick_before_proc;
            snprintf(buffer, sizeof(buffer), "elapsed time: %dms\n", ticks_in_between);
            OutputDebugStringA(buffer);
        }
        SDL_Delay(5);
        
        SDL_RenderPresent(global_renderer);
    }

    SDL_DestroyRenderer(global_renderer);
    SDL_DestroyWindow(Window);
    SDL_Quit();
    return 0;
}
