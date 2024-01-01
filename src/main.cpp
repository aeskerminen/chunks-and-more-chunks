#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <iostream>

#include "chunk.h"
#include "tile.h"
#include "world_system.h"
#include "player.h"
#include "globals.h"

SDL_Window* window = nullptr;
SDL_Surface* surface = nullptr;
SDL_Renderer* renderer = nullptr;

SDL_FRect camera {4 * CHUNK_PIXEL_WIDTH, 4 * CHUNK_PIXEL_WIDTH, SCREEN_WIDTH, SCREEN_HEIGHT};
float cam_vel_x = 0;
float cam_vel_y = 0;

typedef struct item 
{
    Uint8 item_id;

    bool stackable;
    Uint8 count;
} item;

typedef struct inventory 
{
    std::vector<item> contents;
    size_t max_size;
} inventory;

bool initialize() 
{
    bool success = true;


    if(SDL_Init(SDL_INIT_EVERYTHING) < 0) 
    {
        printf("SDL could not intiailize! SDL_Error: %s\n", SDL_GetError());
        success = false;
    }
    
    IMG_Init(IMG_INIT_JPG);
    
    window = SDL_CreateWindow("Terraria", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, 
        SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    
    if(window == nullptr) 
    {
        success = false;
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    return success;
}

void do_render_chunks(const std::vector<chunk> &chunks, player player) 
{
        SDL_PixelFormat* pixel_format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);
        
        SDL_SetRenderDrawColor(renderer, 0,0,0,255);
        SDL_RenderClear(renderer);
        
        SDL_Color colors[3] = 
        {
            {86, 125, 70, 255},
            {155, 118, 83, 255},
            {96, 103, 107, 255},
        };

        for(int k = 0; k < chunks.size(); k++) 
        { 
            SDL_FRect chunkrect {
                chunks[k].x_off_w, 
                chunks[k].y_off_w, 
                CHUNK_SIZE * BLOCK_SIZE * 0.95, 
                CHUNK_SIZE * BLOCK_SIZE * 0.95
            };

            SDL_Rect inter;

            if(SDL_HasIntersectionF(&chunkrect, &camera))
            { 
                for(int i = 0; i < CHUNK_SIZE; i++) 
                {
                    for(int j = 0; j < CHUNK_SIZE; j++) 
                    {
                        Uint32 pixel = chunks[k].arr[i][j].color;
                        Uint8 r,g,b,a;
                        r = 0; g = 0; b = 0; a = 0;
                        
                        SDL_GetRGBA(pixel, pixel_format, &r, &g, &b, &a);
                        
                        Uint8 luminance = (0.2126*r + 0.7152*g + 0.0722*b);

                        if(luminance > (255 / 2)) 
                        {
                            if(luminance < 170)
                                SDL_SetRenderDrawColor(renderer, colors[0].r, colors[0].g ,colors[0].b, a);
                            else if(luminance < 213) 
                                SDL_SetRenderDrawColor(renderer, colors[1].r, colors[1].g ,colors[1].b, a);
                            else
                                SDL_SetRenderDrawColor(renderer, colors[2].r, colors[2].g ,colors[2].b, a);
                        }
                        else
                            SDL_SetRenderDrawColor(renderer, 0, 0, 0, a);

                        SDL_Rect rect {
                            (i * BLOCK_SIZE + chunks[k].x_off_w) - camera.x, 
                            (j * BLOCK_SIZE + chunks[k].y_off_w) - camera.y, 
                            BLOCK_SIZE, 
                            BLOCK_SIZE
                        };
                                
                        SDL_RenderFillRect(renderer, &rect);

                        // DEBUG
                        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                        SDL_RenderDrawRect(renderer, &rect);
                    }
                }
            }
        }
     
        SDL_SetRenderDrawColor(renderer, 255, 5, 255, 255);

        SDL_Rect test {player.x - camera.x, player.y - camera.y, PLAYER_WIDTH, PLAYER_HEIGHT};
        SDL_RenderFillRect(renderer, &test);
}

void do_camera_move(player player, const Uint8* keystate, const float dt) 
{
        cam_vel_x = 0;
        cam_vel_y = 0;
        
        // CAMERA FREE-MOVE
        if(keystate[SDL_SCANCODE_LEFT] == 1)
            cam_vel_x = -25 * dt;
        if(keystate[SDL_SCANCODE_RIGHT] == 1)
            cam_vel_x = 25 * dt;
        if(keystate[SDL_SCANCODE_UP] == 1)
            cam_vel_y = -25 * dt;
        if(keystate[SDL_SCANCODE_DOWN] == 1)
            cam_vel_y = 25 * dt;
        
        if(camera.x + cam_vel_x >= 0 && camera.x + cam_vel_x <= WORLD_CHUNK_W * CHUNK_SIZE * BLOCK_SIZE)
        {    
            camera.x += cam_vel_x;
        }
        if(camera.y + cam_vel_y <= WORLD_CHUNK_H * CHUNK_SIZE * BLOCK_SIZE - SCREEN_HEIGHT 
                && camera.y + cam_vel_y >= 0)
        { 
                camera.y += cam_vel_y;
        }

        SDL_FRect player_rect {player.x - camera.x, player.y - camera.y, PLAYER_WIDTH, PLAYER_HEIGHT};

        camera.x += (player.x - camera.x - SCREEN_WIDTH / 2) * dt * 0.15f;
        camera.y += (player.y - camera.y - SCREEN_HEIGHT / 2) * dt * 0.15f;


}

void get_block_at_cursor(const std::vector<chunk>& chunks) 
{ 
    // Get mouse localtion in world coordinates
    int mx, my;
    SDL_GetMouseState(&mx, &my);

    int mx_world = mx + camera.x;
    int my_world = my + camera.y;

    // Get block at world coordinates
    int m_block_global_x = (mx_world / BLOCK_SIZE);
    int m_block_global_y = (my_world / BLOCK_SIZE);

    int m_block_local_x = m_block_global_x % CHUNK_SIZE;
    int m_block_local_y = m_block_global_y % CHUNK_SIZE;

    // Get chunk from global block position
    int m_block_chunk_index_x = ceil(m_block_global_x / CHUNK_SIZE);
    int m_block_chunk_index_y = ceil(m_block_global_y / CHUNK_SIZE);
    
    // Get actual index
    int m_block_chunk_index = WORLD_CHUNK_W * m_block_chunk_index_y + m_block_chunk_index_x;

    // Reference to block
    auto& block = chunks[m_block_chunk_index].arr[m_block_local_x][m_block_local_y];

    // Remove block 
    SDL_Log("Type: %s, Color: %d, Collider: %s\n", 
            TTypeStrings[block.type], block.color, ColliderStrings[block.col]);
}

void do_show_mouse_helper(const std::vector<chunk>& chunks) 
{
     // Get mouse localtion in world coordinates
    int mx, my;
    SDL_GetMouseState(&mx, &my);

    int mx_world = mx + camera.x;
    int my_world = my + camera.y;

    // Get block at world coordinates
    int m_block_global_x = (mx_world / BLOCK_SIZE);
    int m_block_global_y = (my_world / BLOCK_SIZE);

    int m_block_local_x = m_block_global_x % CHUNK_SIZE;
    int m_block_local_y = m_block_global_y % CHUNK_SIZE;

    // Get chunk from global block position
    int m_block_chunk_index_x = ceil(m_block_global_x / CHUNK_SIZE);
    int m_block_chunk_index_y = ceil(m_block_global_y / CHUNK_SIZE);
    
    // Get actual index
    int m_block_chunk_index = WORLD_CHUNK_W * m_block_chunk_index_y + m_block_chunk_index_x;

    // Reference to block
    auto& block = chunks[m_block_chunk_index].arr[m_block_local_x][m_block_local_y];

    if(block.col != Collider::none) 
    {
        SDL_FRect helper {
            m_block_global_x * BLOCK_SIZE - camera.x, 
            m_block_global_y * BLOCK_SIZE - camera.y, 
            BLOCK_SIZE, 
            BLOCK_SIZE
        };

        SDL_SetRenderDrawColor(renderer, 200, 100,200, 100);
        SDL_RenderDrawRectF(renderer, &helper);
    }
} 

int main(int argc, char* argv[]) 
{
    if(!initialize()) 
        SDL_Quit();

    SDL_Event e; 
    bool quit = false; 
   
    Uint32 lastFrame = SDL_GetTicks();

    std::vector<chunk> chunks = generate_world(WORLD_CHUNK_W, WORLD_CHUNK_H);  
    player player {
        4 * CHUNK_PIXEL_WIDTH - 8 * BLOCK_SIZE, 
        4 * CHUNK_PIXEL_WIDTH - 22 * BLOCK_SIZE, 
        0,
        0, 
        false, 
        {0,0,0}
    };
    
    while(!quit) 
    {
        bool mouse_left_press = false;

        while(SDL_PollEvent(&e)) 
        { 
            if(e.type == SDL_QUIT)    
                quit = true;
            if(e.type == SDL_MOUSEBUTTONDOWN)
                mouse_left_press = true;
        }

        Uint32 curFrame = SDL_GetTicks();
        
        Uint32 difference = curFrame - lastFrame;
		float dt = difference / 25.0f;

		lastFrame = curFrame;

        const Uint8 *keystate = SDL_GetKeyboardState(NULL);

        // RENDER CHUNKS
        do_render_chunks(chunks, player);

        // MOUSE HELPER
        do_show_mouse_helper(chunks);

        // PLAYER COLLISION
        do_player_collision(player, chunks, camera);
      
        // PLAYER
        do_player_move(player, keystate, dt);
      
        // GET BLOCK AT CURSOR (IF CLICKED)
        if(mouse_left_press) 
        {
            get_block_at_cursor(chunks);
            mouse_left_press = false;
        }

        // CAMERA
        do_camera_move(player, keystate, dt);
 
        // Draw
        SDL_RenderPresent(renderer);
        
    }


    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
