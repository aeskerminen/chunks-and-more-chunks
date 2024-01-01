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

const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1200;

SDL_Window* window = nullptr;
SDL_Surface* surface = nullptr;
SDL_Renderer* renderer = nullptr;

const int PLAYER_W_MULT = 2;
const int PLAYER_H_MULT = 3;

constexpr int PLAYER_WIDTH = BLOCK_SIZE*PLAYER_W_MULT;
constexpr int PLAYER_HEIGHT = BLOCK_SIZE*PLAYER_H_MULT;

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

int GRAVITY = 9.81;
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

void do_player_collision(player& player, const std::vector<chunk>& chunks) 
{
        bool& col_l = player.coll[0];
        bool& col_b = player.coll[1];
        bool& col_r = player.coll[2];
        bool& col_t = player.coll[3];
        
        col_l = col_b = col_r = col_t = false;

        auto CIndexFromBlock = [](int bx, int by)
        {
            SDL_Point point;
            point.x = ceil(bx / CHUNK_SIZE);
            point.y = ceil(by / CHUNK_SIZE);

            return point;
        };

        auto GetLocalBlockPoint = [](int bx, int by) 
        {
            SDL_Point point;
            point.x = floor(bx % CHUNK_SIZE);
            point.y = floor(by % CHUNK_SIZE);
            
            return point;
        };

        // PLAYER COLLIDER
        SDL_FRect player_col {(player.x) - camera.x, (player.y) - camera.y, PLAYER_WIDTH, PLAYER_HEIGHT};
        
        // THE BLOCK ON WHICH THE PLAYERS HEAD IS ON
        SDL_Point PHeadBlock {floor(player.x / BLOCK_SIZE), floor(player.y / BLOCK_SIZE)};

        // BOTTOM & TOP COLLISION        
        {

            // BOTTOM
            for(int k = 0; k < PLAYER_W_MULT * 2; k++) 
            {
                int actual_loc = k - 1;
                
                SDL_Point globalCoord {PHeadBlock.x + actual_loc, PHeadBlock.y + PLAYER_H_MULT};
                SDL_Point chunkCoord = CIndexFromBlock(globalCoord.x, globalCoord.y);
                
                int actualChunk = WORLD_CHUNK_W * chunkCoord.y + chunkCoord.x;
                SDL_Point localCoord = GetLocalBlockPoint(globalCoord.x, globalCoord.y);
                
                SDL_FRect collider {
                    globalCoord.x * BLOCK_SIZE - camera.x,
                    globalCoord.y * BLOCK_SIZE - camera.y,
                    BLOCK_SIZE,
                    BLOCK_SIZE
                };
                
                if(SDL_HasIntersectionF(&collider, &player_col) && 
                    chunks[actualChunk].arr[localCoord.x][localCoord.y].col 
                    == Collider::hard)
                    col_b = true;

                SDL_RenderDrawRectF(renderer, &collider);
            }

            // TOP
            for(int k = 0; k < PLAYER_W_MULT * 2; k++) 
            {
                int actual_loc = k - 1;
                
                SDL_Point globalCoord {PHeadBlock.x + actual_loc, PHeadBlock.y - 1};
                SDL_Point chunkCoord = CIndexFromBlock(globalCoord.x, globalCoord.y);
                
                int actualChunk = WORLD_CHUNK_W * chunkCoord.y + chunkCoord.x;
                SDL_Point localCoord = GetLocalBlockPoint(globalCoord.x, globalCoord.y);
                
                SDL_FRect collider {
                    globalCoord.x * BLOCK_SIZE - camera.x,
                    globalCoord.y * BLOCK_SIZE - camera.y,
                    BLOCK_SIZE,
                    BLOCK_SIZE * 1.025f
                };
                
                if(SDL_HasIntersectionF(&collider, &player_col) && 
                    chunks[actualChunk].arr[localCoord.x][localCoord.y].col 
                    == Collider::hard)
                    col_t = true;

                SDL_RenderDrawRectF(renderer, &collider);
            }


        }

        // LEFT & RIGHT COLLISION
        {

            // LEFT
            for(int k = 0; k < PLAYER_H_MULT; k++) 
            {        
                SDL_Point globalCoord {PHeadBlock.x - 1, PHeadBlock.y + k};
                SDL_Point chunkCoord = CIndexFromBlock(globalCoord.x, globalCoord.y);
                
                int actualChunk = WORLD_CHUNK_W * chunkCoord.y + chunkCoord.x;
                SDL_Point localCoord = GetLocalBlockPoint(globalCoord.x, globalCoord.y);
                
                SDL_FRect collider {
                    globalCoord.x * BLOCK_SIZE - camera.x,
                    globalCoord.y * BLOCK_SIZE - camera.y,
                    BLOCK_SIZE * 1.025f,
                    BLOCK_SIZE
                };
                
                if(SDL_HasIntersectionF(&collider, &player_col) && 
                    chunks[actualChunk].arr[localCoord.x][localCoord.y].col 
                    == Collider::hard)
                    col_l = true;

                SDL_RenderDrawRectF(renderer, &collider);
            }

            // RIGHT
            for(int k = 0; k < PLAYER_H_MULT; k++) 
            {        
                SDL_Point globalCoord {PHeadBlock.x + PLAYER_W_MULT, PHeadBlock.y + k};
                SDL_Point chunkCoord = CIndexFromBlock(globalCoord.x, globalCoord.y);
                
                int actualChunk = WORLD_CHUNK_W * chunkCoord.y + chunkCoord.x;
                SDL_Point localCoord = GetLocalBlockPoint(globalCoord.x, globalCoord.y);
                
                SDL_FRect collider {
                    globalCoord.x * BLOCK_SIZE - camera.x,
                    globalCoord.y * BLOCK_SIZE - camera.y,
                    BLOCK_SIZE * 1.025f,
                    BLOCK_SIZE
                };
                
                if(SDL_HasIntersectionF(&collider, &player_col) && 
                    chunks[actualChunk].arr[localCoord.x][localCoord.y].col 
                    == Collider::hard)
                    col_r = true;

                SDL_RenderDrawRectF(renderer, &collider);
            }
        }
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

void do_player_move(player& player, const Uint8* keystate, const float dt) 
{
        bool& lcol = player.coll[0];
        bool& bcol = player.coll[1];
        bool& rcol = player.coll[2];
        bool& tcol = player.coll[3];

        // PLAYER
        if(keystate[SDL_SCANCODE_A] == 1)
            player.velx = -10;
        if(keystate[SDL_SCANCODE_D] == 1)
            player.velx = 10;
        if(keystate[SDL_SCANCODE_S] == 1 && bcol && !tcol && !player.jump) 
        {
            player.vely -= JUMP_FORCE;
            player.jump = true;
        }
        if (keystate[SDL_SCANCODE_S] == 0) { player.jump = false; }

        if(bcol)
            GRAVITY = 0;
        else
            GRAVITY = 9.81;

        player.vely += GRAVITY * dt;
        
        if(player.vely > TERMINAL_VELOCITY)
            player.vely = TERMINAL_VELOCITY;
        if(tcol)
            player.vely = GRAVITY * dt;

        // Add motion to player
        if(!bcol)
            player.y += player.vely * dt;
        else if(bcol && player.vely < 0)
            player.y += player.vely * dt;

        if(!lcol && player.velx < 0)
            player.x += player.velx * dt;
        if(!rcol && player.velx > 0)
            player.x += player.velx * dt;

        player.velx = 0; 
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
        do_player_collision(player, chunks);
      
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