#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <iostream>
#include <stdlib.h>

#define NK_IMPLEMENTATION
#define NK_SDL_RENDERER_IMPLEMENTATION

#include "chunk.h"
#include "tile.h"
#include "world_system.h"
#include "player.h"
#include "globals.h"
#include "item.h"
#include "inventory.h"
#include "camera.h"
#include "pointer.h"
#include "ui.h"
#include "util.h"

SDL_Window *window = nullptr;
SDL_Surface *surface = nullptr;
SDL_Renderer *renderer = nullptr;

bool initialize()
{
    bool success = true;

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        success = false;
    }

    IMG_Init(IMG_INIT_JPG);

    window = SDL_CreateWindow("Terraria", SDL_WINDOW_SHOWN, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                              SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

    if (window == nullptr)
    {
        success = false;
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    return success;
}

void do_render_chunks(const std::vector<chunk> &chunks, player player)
{
    SDL_PixelFormat *pixel_format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);

    SDL_Color colors[3] =
        {
            {86, 125, 70, 255},
            {155, 118, 83, 255},
            {96, 103, 107, 255},
        };

    for (int k = 0; k < chunks.size(); k++)
    {
        SDL_FRect chunkrect{
            chunks[k].x_off_w,
            chunks[k].y_off_w,
            CHUNK_SIZE * BLOCK_SIZE * 0.95,
            CHUNK_SIZE * BLOCK_SIZE * 0.95};

        SDL_Rect inter;

        if (SDL_HasIntersectionF(&chunkrect, &camera))
        {
            for (int i = 0; i < CHUNK_SIZE; i++)
            {
                for (int j = 0; j < CHUNK_SIZE; j++)
                {
                    Uint32 pixel = chunks[k].arr[i][j].color;
                    Uint8 r, g, b, a;
                    r = g = b = a = 0;

                    SDL_GetRGBA(pixel, pixel_format, &r, &g, &b, &a);

                    Uint8 luminance = (0.2126 * r + 0.7152 * g + 0.0722 * b);

                    if (luminance > (255 / 2))
                    {
                        if (luminance < 170)
                            SDL_SetRenderDrawColor(renderer, colors[0].r, colors[0].g, colors[0].b, a);
                        else if (luminance < 213)
                            SDL_SetRenderDrawColor(renderer, colors[1].r, colors[1].g, colors[1].b, a);
                        else
                            SDL_SetRenderDrawColor(renderer, colors[2].r, colors[2].g, colors[2].b, a);

                        SDL_Rect rect{
                            (i * BLOCK_SIZE + chunks[k].x_off_w) - camera.x,
                            (j * BLOCK_SIZE + chunks[k].y_off_w) - camera.y,
                            BLOCK_SIZE,
                            BLOCK_SIZE};

                        SDL_RenderFillRect(renderer, &rect);

                        // DEBUG
                        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                        SDL_RenderDrawRect(renderer, &rect);
                    }
                }
            }
        }
    }

    SDL_SetRenderDrawColor(renderer, 255, 5, 255, 255);

    SDL_Rect test{player.x - camera.x, player.y - camera.y, PLAYER_WIDTH, PLAYER_HEIGHT};
    SDL_RenderFillRect(renderer, &test);
}

void do_render_items(const std::vector<item> &items)
{
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 100);
    for (const item i : items)
    {
        SDL_Rect item_rect = SDL_Rect{i.x - camera.x, i.y - camera.y, ITEM_SIZE, ITEM_SIZE};
        SDL_RenderFillRect(renderer, &item_rect);
    }
}

int WinMain(int argc, char *argv[])
{
    if (!initialize())
        SDL_Quit();

    init_ui(window, renderer);

    SDL_Event e;
    bool quit = false;

    Uint32 lastFrame = SDL_GetTicks();

    std::vector<chunk> chunks = generate_world(WORLD_CHUNK_W, WORLD_CHUNK_H);

    std::vector<item> items;

    player player{
        4 * CHUNK_PIXEL_WIDTH - 8 * BLOCK_SIZE,
        4 * CHUNK_PIXEL_WIDTH - 22 * BLOCK_SIZE,
        0,
        0,
        false,
        {0, 0, 0}};
    player.inv.max_size = 40;

    const item test_item{1, player.x, player.y, false, 1};

    items.push_back(test_item);

    tile empty_tile{TType::air, 0, Collider::none};

    const float GROUND_LEVEL = player.y - PLAYER_HEIGHT - 10; // Set ground level just below spawn.

    Uint64 cur_frame = SDL_GetPerformanceCounter();
    Uint64 last_frame = 0;

    double dt = 0;

    float speedY = 0.0f;
    float speedX = 0.0f;

    float speedMX = 0.6f;
    float speedMY = 1.9f;

    float friction = 0.95f;
    float runSpeed = 0.225f;

    float jumpForce = 6.0f;

    int right = 0, left = 0;

    int grounded = 0;

    int up = 0;

    while (!quit)
    {
        bool mouse_left_press = false;

        nk_input_begin(ctx);
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                quit = true;
            if (e.type == SDL_MOUSEBUTTONDOWN)
                mouse_left_press = true;

            switch (e.type)
            {
            case SDL_KEYDOWN:
            {
                switch (e.key.keysym.sym)
                {
                case SDLK_a:
                    left = 1;
                    break;
                case SDLK_d:
                    right = 1;
                    break;
                case SDLK_SPACE:
                    up = 1;
                    break;
                }
                break;
            }
            case SDL_KEYUP:
            {
                switch (e.key.keysym.sym)
                {
                case SDLK_a:
                    left = 0;
                    break;
                case SDLK_d:
                    right = 0;
                    break;
                case SDLK_SPACE:
                    up = 0;
                    break;
                }
                break;  
            }
            }

            nk_sdl_handle_event(&e);
        }
        nk_input_end(ctx);

        last_frame = cur_frame;
        cur_frame = SDL_GetPerformanceCounter();

        dt = 1;//1 / ((double)((cur_frame - last_frame) * 1000 / (double)SDL_GetPerformanceFrequency()));

        const Uint8 *keystate = SDL_GetKeyboardState(NULL);

        // CLEAR
        SDL_SetRenderDrawColor(renderer, 180, 235, 240, 255);
        SDL_RenderClear(renderer);

        // RENDER CHUNKS
        do_render_chunks(chunks, player);

        // RENDER ITEMS
        do_render_items(items);

        // ITEM LOGIC
        do_tick_items(items, chunks, dt);

        do_player_collision(player, chunks, camera);

        bool col_l = player.coll[0];
        bool col_b = player.coll[1];
        bool col_r = player.coll[2];
        bool col_t = player.coll[3];

        // PLAYER MOVEMENT
        speedX += ((right * runSpeed) - (left * runSpeed)) * dt;
        speedX *= friction;

        if ((speedX < 0 && !col_l) || (speedX > 0 && !col_r))
        {
            player.x += std::clamp(speedX, -speedMX, speedMX);
        }

        speedY -= (((col_t ? 0 : 1) * grounded * up * jumpForce) - GRAVITY) * dt; // Gravity increases speedY downward.

        float before = player.y;
        player.y += std::clamp(speedY, -speedMY, speedMY);

        if (col_b && speedY > 0)
        {
            speedY = 0;
            player.y = before;
            grounded = 1;
        }
        else
        {
            grounded = 0;
        }

        if (col_t && speedY < 0)
        {
            speedY = 0;
            player.y = before;
        }

        // MOUSE
        do_show_mouse_helper(chunks, renderer);

        // GET BLOCK AT CURSOR (IF CLICKED)
        if (mouse_left_press)
        {
            tile *p = get_block_at_cursor(chunks);
            mouse_left_press = false;
        }

        // CAMERA
        do_camera_move(player.x, player.y, keystate, dt);

        // GUI GUI GUI
        render_inventory(player);

        // Draw
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
