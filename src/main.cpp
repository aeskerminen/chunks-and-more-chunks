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
#include "item.h"
#include "inventory.h"


#define NK_IMPLEMENTATION
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_RENDERER_IMPLEMENTATION
#include "nuklear.h"
#include "../nuklear_sdl_renderer.h"

SDL_Window* window = nullptr;
SDL_Surface* surface = nullptr;
SDL_Renderer* renderer = nullptr;

SDL_FRect camera {4 * CHUNK_PIXEL_WIDTH, 4 * CHUNK_PIXEL_WIDTH, SCREEN_WIDTH, SCREEN_HEIGHT};
float cam_vel_x = 0;
float cam_vel_y = 0;

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

        const float cam_base_vel = 0.75f;

        cam_vel_x = 0;
        cam_vel_y = 0;
        
        // CAMERA FREE-MOVE
        if(keystate[SDL_SCANCODE_LEFT] == 1)
            cam_vel_x = -cam_base_vel;
        if(keystate[SDL_SCANCODE_RIGHT] == 1)
            cam_vel_x = cam_base_vel;
        if(keystate[SDL_SCANCODE_UP] == 1)
            cam_vel_y = -cam_base_vel;
        if(keystate[SDL_SCANCODE_DOWN] == 1)
            cam_vel_y = cam_base_vel;
        
        if(camera.x + cam_vel_x <= WORLD_CHUNK_W * CHUNK_SIZE * BLOCK_SIZE)
        {    
            camera.x += cam_vel_x * dt;
        }
        if(camera.y + cam_vel_y <= WORLD_CHUNK_H * CHUNK_SIZE * BLOCK_SIZE - SCREEN_HEIGHT 
                && camera.y + cam_vel_y >= 0)
        { 
                camera.y += cam_vel_y * dt;
        }

        SDL_FRect player_rect {player.x - camera.x, player.y - camera.y, PLAYER_WIDTH, PLAYER_HEIGHT};

        camera.x += (player.x - camera.x - SCREEN_WIDTH / 2) * dt * 0.005f;
        camera.y += (player.y - camera.y - SCREEN_HEIGHT / 2) * dt * 0.005f;
}

tile* get_block_at_cursor(std::vector<chunk>& chunks) 
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

    return &chunks[m_block_chunk_index].arr[m_block_local_x][m_block_local_y];
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

    /* GUI */
    struct nk_context *ctx;
    struct nk_colorf bg;
    float font_scale = 1;
    
    // NUKLEAR NUKLEAR NUKLEAR
    
    /* GUI */
    ctx = nk_sdl_init(window, renderer);
    /* Load Fonts: if none of these are loaded a default font will be used  */
    /* Load Cursor: if you uncomment cursor loading please hide the cursor */
    {
        struct nk_font_atlas *atlas;
        struct nk_font_config config = nk_font_config(0);
        struct nk_font *font;

        /* set up the font atlas and add desired font; note that font sizes are
         * multiplied by font_scale to produce better results at higher DPIs */
        nk_sdl_font_stash_begin(&atlas);
        font = nk_font_atlas_add_default(atlas, 13 * font_scale, &config);
        /*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/DroidSans.ttf", 14 * font_scale, &config);*/
        /*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Roboto-Regular.ttf", 16 * font_scale, &config);*/
        /*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/kenvector_future_thin.ttf", 13 * font_scale, &config);*/
        /*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyClean.ttf", 12 * font_scale, &config);*/
        /*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyTiny.ttf", 10 * font_scale, &config);*/
        /*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Cousine-Regular.ttf", 13 * font_scale, &config);*/
        nk_sdl_font_stash_end();

        /* this hack makes the font appear to be scaled down to the desired
         * size and is only necessary when font_scale > 1 */
        font->handle.height /= font_scale;
        /*nk_style_load_all_cursors(ctx, atlas->cursors);*/
        nk_style_set_font(ctx, &font->handle);
    }

    #ifdef INCLUDE_STYLE
    /* ease regression testing during Nuklear release process; not needed for anything else */
    #ifdef STYLE_WHITE
    set_style(ctx, THEME_WHITE);
    #elif defined(STYLE_RED)
    set_style(ctx, THEME_RED);
    #elif defined(STYLE_BLUE)
    set_style(ctx, THEME_BLUE);
    #elif defined(STYLE_DARK)
    set_style(ctx, THEME_DARK);
    #endif
    #endif   

    // NUKLEAR NUKLEAR NUKLEAR


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
    player.inv.max_size = 40;

    tile empty_tile {TType::air, 0, Collider::none};
    
    Uint64 cur_frame = SDL_GetPerformanceCounter();
    Uint64 last_frame = 0;
    
    double dt = 0;

    while(!quit) 
    { 
        bool mouse_left_press = false;
        
        nk_input_begin(ctx);
        while(SDL_PollEvent(&e)) 
        { 
            if(e.type == SDL_QUIT)    
                quit = true;
            if(e.type == SDL_MOUSEBUTTONDOWN)
                mouse_left_press = true;

            nk_sdl_handle_event(&e);
        }
        nk_input_end(ctx);

        last_frame = cur_frame;
        cur_frame = SDL_GetPerformanceCounter();

        dt = (double)((cur_frame - last_frame) * 1000 / (double)SDL_GetPerformanceFrequency());

        //SDL_Log("%f\n", dt);

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
            tile* p = get_block_at_cursor(chunks);
            
            item collected_item {p->type, false, 1};
            player.inv.contents.push_back(collected_item);

            *p = empty_tile;
            
            mouse_left_press = false;

            for(auto x : player.inv.contents)
            {
                SDL_Log("%d %d %d", x.item_id, x.stackable, x.count);
            }
        }

        // CAMERA
        do_camera_move(player, keystate, dt);
 

        // GUI GUI GUI
        
        /* GUI */
        if (nk_begin(ctx, "Demo", nk_rect(50, 50, 230, 250),
            NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
            NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
        {
            enum {EASY, HARD};
            static int op = EASY;
            static int property = 20;

            nk_layout_row_static(ctx, 30, 80, 1);
            if (nk_button_label(ctx, "button"))
                fprintf(stdout, "button pressed\n");
            nk_layout_row_dynamic(ctx, 30, 2);
            if (nk_option_label(ctx, "easy", op == EASY)) op = EASY;
            if (nk_option_label(ctx, "hard", op == HARD)) op = HARD;
            nk_layout_row_dynamic(ctx, 25, 1);
            nk_property_int(ctx, "Compression:", 0, &property, 100, 10, 1);

            nk_layout_row_dynamic(ctx, 20, 1);
            nk_label(ctx, "background:", NK_TEXT_LEFT);
            nk_layout_row_dynamic(ctx, 25, 1);
            if (nk_combo_begin_color(ctx, nk_rgb_cf(bg), nk_vec2(nk_widget_width(ctx),400))) {
                nk_layout_row_dynamic(ctx, 120, 1);
                bg = nk_color_picker(ctx, bg, NK_RGBA);
                nk_layout_row_dynamic(ctx, 25, 1);
                bg.r = nk_propertyf(ctx, "#R:", 0, bg.r, 1.0f, 0.01f,0.005f);
                bg.g = nk_propertyf(ctx, "#G:", 0, bg.g, 1.0f, 0.01f,0.005f);
                bg.b = nk_propertyf(ctx, "#B:", 0, bg.b, 1.0f, 0.01f,0.005f);
                bg.a = nk_propertyf(ctx, "#A:", 0, bg.a, 1.0f, 0.01f,0.005f);
                nk_combo_end(ctx);
            }
        }
        nk_end(ctx);

        nk_sdl_render(NK_ANTI_ALIASING_ON);
        // GUI GUI GUI
        
        // Draw
        SDL_RenderPresent(renderer);

    }


    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
