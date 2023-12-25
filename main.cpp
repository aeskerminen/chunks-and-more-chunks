#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <vector>

const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1200;

SDL_Window* window = nullptr;
SDL_Surface* surface = nullptr;
SDL_Renderer* renderer = nullptr;

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

#define PLAYER_WIDTH BLOCK_SIZE
#define PLAYER_HEIGHT BLOCK_SIZE*2

#define BLOCK_SIZE 40
#define CHUNK_SIZE 32

const int WORLD_CHUNK_W = 8;
const int WORLD_CHUNK_H = 8;

constexpr int CHUNK_RES = BLOCK_SIZE * CHUNK_SIZE;
constexpr int WOLRD_RES = WORLD_CHUNK_W * WORLD_CHUNK_H * CHUNK_RES * CHUNK_RES;

// DEBUG LOCATION
SDL_FRect camera {4 * CHUNK_RES, 4 * CHUNK_RES, SCREEN_WIDTH, SCREEN_HEIGHT};
float cam_vel_x = 0;
float cam_vel_y = 0;

#define JUMP_FORCE 50
typedef struct player 
{
    float x, y;
    float velx, vely;
} player;

enum Collider {hard, soft, none}; 
typedef struct tile 
{
    char type;
    Uint32 color;
    Collider col;
} tile;

typedef struct chunk 
{
    tile arr[CHUNK_SIZE][CHUNK_SIZE];
    int x_off_w;
    int y_off_w;
    bool change;
} chunk;

std::vector<chunk> generate_world(int w_width, int w_height) 
{
    auto noise = IMG_Load("./samplenoise.png");
    Uint32* pixels = (Uint32*)noise->pixels;

    size_t noise_w = noise->w;
    size_t noise_h = noise->h;

    Uint32** textureBuffer = (Uint32**)malloc(noise_h * sizeof(Uint32*));
    for (int i = 0; i < noise_h; i++)
        textureBuffer[i] = (Uint32*)malloc(noise_w * sizeof(Uint32));

    for(int y = 0; y < noise_h; y++)
        for(int x = 0; x < noise_w; x++)
            textureBuffer[x][y] = pixels[noise_w * y + x];

    SDL_PixelFormat *formatPix = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);    
    std::vector<chunk> chunks;

    int height = 0;
    for(int k = 0; k < w_width * w_height; k++) 
    {
        chunk c;
        c.x_off_w = CHUNK_RES * (k % w_width);
        c.y_off_w = CHUNK_RES * height;
    
        chunks.push_back(c);

        if((k+1) % w_height == 0 && k != 0)
            height++;
    }

    for(int y = 0; y < noise_h; y++) 
    {
        for(int x = 0; x < noise_w; x++) 
        {
            int cindex = WORLD_CHUNK_W * ceil(y / CHUNK_SIZE) + ceil(x / CHUNK_SIZE);
            chunks[cindex].arr[x % 32][y % 32].color = textureBuffer[x][y];   
        }
    }
 
    return chunks;
}

void do_player_collision(player player, const std::vector<chunk>& chunks,  bool* col_b, bool* col_l, bool* col_r) 
{
        *col_b = false;
        *col_l = false;
        *col_r = false;
        
        // PLAYER COLLIDER
        SDL_FRect player_col {(player.x) - camera.x, (player.y) - camera.y, PLAYER_WIDTH, PLAYER_HEIGHT};
        
        // THE BLOCK ON WHICH THE PLAYERS HEAD IS ON
        int block_x_global = floor(player.x / BLOCK_SIZE);
        int block_y_global = floor(player.y / BLOCK_SIZE);
        
        // BOTTOM COLLISION        
        {

            SDL_Point local_points[3] = 
            {
                {block_x_global % CHUNK_SIZE, (block_y_global + 2) % CHUNK_SIZE},
                {(block_x_global + 1) % CHUNK_SIZE, (block_y_global + 2) % CHUNK_SIZE},
                {(block_x_global - 1) % CHUNK_SIZE, (block_y_global + 2) % CHUNK_SIZE}
            };

            int target_chunks[3] = 
            {
                WORLD_CHUNK_W * ceil((block_y_global + 2) / CHUNK_SIZE) + ceil(block_x_global / CHUNK_SIZE),
                WORLD_CHUNK_W * ceil((block_y_global + 2) / CHUNK_SIZE) + ceil((block_x_global + 1) / CHUNK_SIZE),
                WORLD_CHUNK_W * ceil((block_y_global + 2) / CHUNK_SIZE) + ceil((block_x_global - 1) / CHUNK_SIZE) 
            };
            
            SDL_FRect target_colliders[3] = 
            {
                {
                     (block_x_global * BLOCK_SIZE) - camera.x,
                    ((block_y_global + 2) * BLOCK_SIZE) - camera.y,
                    BLOCK_SIZE,
                    BLOCK_SIZE
                },
                {
                    ((block_x_global + 1) * BLOCK_SIZE) - camera.x,
                    ((block_y_global + 2) * BLOCK_SIZE) - camera.y,
                    BLOCK_SIZE,
                    BLOCK_SIZE
                },
                {
                    ((block_x_global - 1) * BLOCK_SIZE) - camera.x,
                    ((block_y_global + 2) * BLOCK_SIZE) - camera.y,
                    BLOCK_SIZE,
                    BLOCK_SIZE
                },
            };

            for(int i = 0; i < 3; i++) 
            {
                if(SDL_HasIntersectionF(&target_colliders[i], &player_col) && 
                    chunks[target_chunks[i]].arr[local_points[i].x][local_points[i].y].col 
                    == Collider::hard)
                    *col_b = true;
            }  

        }

        // LEFT & RIGHT COLLISION
        {

            SDL_Point local_points[4] = 
            {
                {(block_x_global - 1) % CHUNK_SIZE, (block_y_global) % CHUNK_SIZE},
                {(block_x_global - 1) % CHUNK_SIZE, (block_y_global + 1) % CHUNK_SIZE},
                {(block_x_global + 1) % CHUNK_SIZE, (block_y_global) % CHUNK_SIZE},
                {(block_x_global + 1) % CHUNK_SIZE, (block_y_global + 1) % CHUNK_SIZE}
            };

            int target_chunks[4] = 
            {
                WORLD_CHUNK_W * ceil((block_y_global) / CHUNK_SIZE) + ceil((block_x_global - 1) / CHUNK_SIZE),
                WORLD_CHUNK_W * ceil((block_y_global + 1) / CHUNK_SIZE) + ceil((block_x_global - 1) / CHUNK_SIZE),
                WORLD_CHUNK_W * ceil((block_y_global) / CHUNK_SIZE) + ceil((block_x_global + 1) / CHUNK_SIZE),
                WORLD_CHUNK_W * ceil((block_y_global + 1) / CHUNK_SIZE) + ceil((block_x_global + 1) / CHUNK_SIZE),
            
            };
            
            SDL_FRect target_colliders[4] = 
            {
                {
                    ((block_x_global - 1) * BLOCK_SIZE) - camera.x,
                    ((block_y_global) * BLOCK_SIZE) - camera.y,
                    BLOCK_SIZE * 1.025,
                    BLOCK_SIZE
                },
                {
                    ((block_x_global - 1) * BLOCK_SIZE) - camera.x,
                    ((block_y_global + 1) * BLOCK_SIZE) - camera.y,
                    BLOCK_SIZE * 1.025,
                    BLOCK_SIZE
                },
                {
                    ((block_x_global + 1) * BLOCK_SIZE) - camera.x,
                    ((block_y_global) * BLOCK_SIZE) - camera.y,
                    BLOCK_SIZE * 1.025,
                    BLOCK_SIZE
                },
                {
                    ((block_x_global + 1) * BLOCK_SIZE) - camera.x,
                    ((block_y_global + 1) * BLOCK_SIZE) - camera.y,
                    BLOCK_SIZE * 1.025,
                    BLOCK_SIZE
                }
            };


            SDL_SetRenderDrawColor(renderer, 255, 0, 100, 255);
            for(int i = 0; i < 4; i++) 
            {
                if(SDL_HasIntersectionF(&target_colliders[i], &player_col) && 
                    chunks[target_chunks[i]].arr[local_points[i].x][local_points[i].y].col 
                    == Collider::hard) 
                { 
                    if(i < 2)
                        *col_l = true;
                    else
                        *col_r = true;
                }

                SDL_RenderDrawRectF(renderer, &target_colliders[i]);
            }

        }
}

int main(int argc, char* argv[]) 
{
    if(!initialize()) 
        SDL_Quit();

    SDL_Event e; 
    bool quit = false; 
   
    std::vector<chunk> chunks = generate_world(WORLD_CHUNK_W, WORLD_CHUNK_H); 
    
    player player {4 * CHUNK_RES, 4 * CHUNK_RES, 0, 0};
    bool jump = false;


    Uint32 lastFrame = SDL_GetTicks();

    int GRAVITY = 9.81;

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

        // RENDER

        SDL_PixelFormat* pixel_format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);
        
        SDL_SetRenderDrawColor(renderer, 0,0,0,255);
        SDL_RenderClear(renderer);
        
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
                            SDL_SetRenderDrawColor(renderer, r, g,b, a);
                        else
                            SDL_SetRenderDrawColor(renderer, r,g,b, a);
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


        // LOGIC
        const Uint8 *keystate = SDL_GetKeyboardState(NULL);

        cam_vel_x = 0;
        cam_vel_y = 0;

        bool player_b_col, player_l_col, player_r_col;
        do_player_collision(player, chunks, &player_b_col, &player_l_col, &player_r_col);

        // PLAYER
        player.vely = 9.81;
        
        if(keystate[SDL_SCANCODE_A] == 1)
            player.velx = -10;
        if(keystate[SDL_SCANCODE_D] == 1)
            player.velx = 10;
        if(keystate[SDL_SCANCODE_S] == 1 && player_b_col && !jump) 
        {
            player.vely -= JUMP_FORCE * 1500 * dt;
            jump = true;
        }
        else if (keystate[SDL_SCANCODE_S] == 0) { jump = false; }

        if(player_b_col)
            GRAVITY = 0;
        else
            GRAVITY = 9.81 * 2;

        player.vely += GRAVITY * dt;

        // Add motion to player
        if(!player_b_col)
            player.y += player.vely * dt;
        else if(player_b_col && player.vely < 0)
            player.y += player.vely * dt;

        if(!player_l_col && player.velx < 0)
            player.x += player.velx * dt;
        if(!player_r_col && player.velx > 0)
            player.x += player.velx * dt;

        player.velx = 0;
        
        // REMOVING BLOCK (DEBUG STAGE
        
        // Get mouse localtion in world coordinates
        int mx, my;
        SDL_GetMouseState(&mx, &my);

        int mx_world = mx + camera.x;
        int my_world = my + camera.y;

        //SDL_Log("%d / %d", mx_world, my_world);

        SDL_Rect DEBUG_MOUSE 
        {
            mx,
            my,
            15,
            15
        };


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

        // Remove block
        if(mouse_left_press) 
        {
            chunks[m_block_chunk_index].arr[m_block_local_x][m_block_local_y].col = Collider::none; 
            chunks[m_block_chunk_index].arr[m_block_local_x][m_block_local_y].color= 0;  
            mouse_left_press = false;
        }


        // CAMERA
        if(keystate[SDL_SCANCODE_LEFT] == 1)
            cam_vel_x = -25 * dt;
        if(keystate[SDL_SCANCODE_RIGHT] == 1)
            cam_vel_x = 25 * dt;
        if(keystate[SDL_SCANCODE_UP] == 1)
            cam_vel_y = -25 * dt;
        if(keystate[SDL_SCANCODE_DOWN] == 1)
            cam_vel_y = 25 * dt;
        
        if(camera.x + cam_vel_x >= 0 && camera.x + cam_vel_x <= WORLD_CHUNK_W * CHUNK_SIZE * BLOCK_SIZE)
            camera.x += cam_vel_x;
        if(camera.y + cam_vel_y <= WORLD_CHUNK_H * CHUNK_SIZE * BLOCK_SIZE - SCREEN_HEIGHT 
                && camera.y + cam_vel_y >= 0)
            camera.y += cam_vel_y;

        SDL_RenderPresent(renderer);
        
    }


    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
