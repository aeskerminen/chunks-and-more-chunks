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

static const int  SEED = 1985;
    
static const unsigned char  HASH[] = {
    208,34,231,213,32,248,233,56,161,78,24,140,71,48,140,254,245,255,247,247,40,
    185,248,251,245,28,124,204,204,76,36,1,107,28,234,163,202,224,245,128,167,204,
    9,92,217,54,239,174,173,102,193,189,190,121,100,108,167,44,43,77,180,204,8,81,
    70,223,11,38,24,254,210,210,177,32,81,195,243,125,8,169,112,32,97,53,195,13,
    203,9,47,104,125,117,114,124,165,203,181,235,193,206,70,180,174,0,167,181,41,
    164,30,116,127,198,245,146,87,224,149,206,57,4,192,210,65,210,129,240,178,105,
    228,108,245,148,140,40,35,195,38,58,65,207,215,253,65,85,208,76,62,3,237,55,89,
    232,50,217,64,244,157,199,121,252,90,17,212,203,149,152,140,187,234,177,73,174,
    193,100,192,143,97,53,145,135,19,103,13,90,135,151,199,91,239,247,33,39,145,
    101,120,99,3,186,86,99,41,237,203,111,79,220,135,158,42,30,154,120,67,87,167,
    135,176,183,191,253,115,184,21,233,58,129,233,142,39,128,211,118,137,139,255,
    114,20,218,113,154,27,127,246,250,1,8,198,250,209,92,222,173,21,88,102,219
};
    
static int noise2(int x, int y)
{
    int  yindex = (y + SEED) % 256;
    if (yindex < 0)
        yindex += 256;
    int  xindex = (HASH[yindex] + x) % 256;
    if (xindex < 0)
        xindex += 256;
    const int  result = HASH[xindex];
    return result;
}
    
static double lin_inter(double x, double y, double s)
{
    return x + s * (y-x);
}
    
static double smooth_inter(double x, double y, double s)
{
    return lin_inter( x, y, s * s * (3-2*s) );
}
    
static double noise2d(double x, double y)
{
    const int  x_int = floor( x );
    const int  y_int = floor( y );
    const double  x_frac = x - x_int;
    const double  y_frac = y - y_int;
    const int  s = noise2( x_int, y_int );
    const int  t = noise2( x_int+1, y_int );
    const int  u = noise2( x_int, y_int+1 );
    const int  v = noise2( x_int+1, y_int+1 );
    const double  low = smooth_inter( s, t, x_frac );
    const double  high = smooth_inter( u, v, x_frac );
    const double  result = smooth_inter( low, high, y_frac );
    return result;
}
    
double perlin2d(double x, double y, double freq, int depth)
{
    double  xa = x*freq;
    double  ya = y*freq;
    double  amp = 1.0;
    double  fin = 0;
    double  div = 0.0;
    for (int i=0; i < depth; i++)
    {
        div += 256 * amp;
        fin += noise2d( xa, ya ) * amp;
        amp /= 2;
        xa *= 2;
        ya *= 2;
    }
    return fin/div;
}
    
SDL_Color pickColor(int val)
{
    SDL_Color arrColors[] = {
        SDL_Color{40,40,40,255}, // black
        SDL_Color{41,54,111,255}, // dark blue
        SDL_Color{59,93,201,255},
        SDL_Color{64,166,245,255},
        SDL_Color{114,239,247,255}, // light blue
        SDL_Color{148,175,194}, // light grey
        SDL_Color{86,108,134} // dark grey
    };
    
    return arrColors[val];
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
SDL_Rect camera {4 * CHUNK_RES, 4 * CHUNK_RES, SCREEN_WIDTH, SCREEN_HEIGHT};
float cam_vel_x = 0;
float cam_vel_y = 0;

typedef struct player 
{
    int x, y;
    int velx, vely;
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
    int res_x = w_width * CHUNK_SIZE;
    int res_y = w_height * CHUNK_SIZE;
    
    int xOrg = 0;
    int yOrg = 0;
    float freq = 0.8f;
    int depth = 5;
    int scale = 50;
    

    SDL_PixelFormat *formatPix = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);    
    Uint32 pixels[res_x][res_y];
   for (int x = 0; x < res_x; x++) {
        for (int y = 0; y < res_y; y++) {
            float xCoord = xOrg + x / ((float)SCREEN_WIDTH) * scale;
            float yCoord = yOrg + y / ((float)SCREEN_HEIGHT) * scale;
            float value1 = perlin2d(yCoord, xCoord, freq, depth);
            
            SDL_Color col = pickColor((int)(value1*16)%7);
            
            pixels[x][y] = SDL_MapRGBA(formatPix, col.r, col.g, col.b, 255);    
        }
    }

    std::vector<chunk> chunks;

    for(int k = 0; k < w_width * w_height; k++) 
    {
        chunk c;
        c.x_off_w = CHUNK_SIZE * BLOCK_SIZE * (k % w_width);
        c.y_off_w = CHUNK_SIZE * BLOCK_SIZE * (k / w_height);

        for(int i = 0; i < CHUNK_SIZE; i++) 
        {
            for(int j = 0; j < CHUNK_SIZE; j++) 
            {
                if(k >= 32 && k < 40)
                {
                    c.arr[i][j].col = Collider::none;
                    c.arr[i][j].color = SDL_MapRGBA(formatPix, 0, 0, 0, 255);
                }
                else {
                    c.arr[i][j].col = Collider::hard;
                    c.arr[i][j].color = pixels[(i * (k + 1)) % w_width][(j * (k / w_height))];
                }
            }
        }

        chunks.push_back(c);
    }


    return chunks;
}

void do_player_collision(player player, const std::vector<chunk>& chunks,  bool* col_b, bool* col_l, bool* col_r) 
{
        *col_b= false;
        *col_l = false;
        *col_r = false;
        
        // PLAYER COLLIDER
        SDL_Rect player_col {(player.x) - camera.x, (player.y) - camera.y, PLAYER_WIDTH, PLAYER_HEIGHT};
        
        // THE BLOCK ON WHICH THE PLAYERS HEAD IS ON
        int block_x_global = floor(player.x / BLOCK_SIZE);
        int block_y_global = floor(player.y / BLOCK_SIZE);
        
        int block_y_global_bottom = block_y_global + 2;

        // Get chunk from global block position
        int block_chunk_index_x = ceil(block_x_global / CHUNK_SIZE);
        int block_chunk_index_y = ceil(block_y_global_bottom / CHUNK_SIZE);
        
        // CHUNK INDEX
        int block_chunk_index = WORLD_CHUNK_W * block_chunk_index_y + block_chunk_index_x;
       
        // LOCAL BLOCKS FOR BOTTOM
        int block_x_local = block_x_global % CHUNK_SIZE;
        int block_y_local_bottom = block_y_global_bottom % CHUNK_SIZE;


        // BOTTOM COLLISION
        
        SDL_Point local_bottom_points[3] = 
        {
            {block_x_global % CHUNK_SIZE, (block_y_global + 2) % CHUNK_SIZE},
            {(block_x_global + 1) % CHUNK_SIZE, (block_y_global + 2) % CHUNK_SIZE},
            {(block_x_global - 1) % CHUNK_SIZE, (block_y_global + 2) % CHUNK_SIZE}
        };

        int bottom_chunks[3] = 
        {
            WORLD_CHUNK_W * ceil((block_y_global + 2) / CHUNK_SIZE) + ceil(block_x_global / CHUNK_SIZE),
            WORLD_CHUNK_W * ceil((block_y_global + 2) / CHUNK_SIZE) + ceil((block_x_global + 1) / CHUNK_SIZE),
            WORLD_CHUNK_W * ceil((block_y_global + 2) / CHUNK_SIZE) + ceil((block_x_global - 1) / CHUNK_SIZE) 
        };
        
        SDL_Rect bottom_colliders[3] = 
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
            if(SDL_HasIntersection(&bottom_colliders[i], &player_col) && 
                chunks[bottom_chunks[i]].arr[local_bottom_points[i].x][local_bottom_points[i].y].col 
                == Collider::hard)
                *col_b = true;
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

    while(!quit) 
    {
        Uint64 start = SDL_GetPerformanceCounter();

        bool mouse_left_press = false;

        while(SDL_PollEvent(&e)) 
        { 
            if(e.type == SDL_QUIT)    
                quit = true;
            if(e.type == SDL_MOUSEBUTTONDOWN)
                mouse_left_press = true;
        }

        const Uint8 *keystate = SDL_GetKeyboardState(NULL);

        cam_vel_x = 0;
        cam_vel_y = 0;

        bool player_b_col, player_l_col, player_r_col;
        do_player_collision(player, chunks, &player_b_col, &player_l_col, &player_r_col);

        // PLAYER
        if(keystate[SDL_SCANCODE_A] == 1)
            player.velx = -5;
        if(keystate[SDL_SCANCODE_D] == 1)
            player.velx = 5;
    
        player.vely = 9.81;

        // Add motion to player
        if(!player_b_col)
            player.y += player.vely;
        if(!player_l_col && player.velx < 0)
            player.x += player.velx;
        if(!player_r_col && player.velx > 0)
            player.x += player.velx;

        player.velx = 0;

       

       /* 
        SDL_Rect DEBUG_BLOCK 
        {
            (block_x_global * BLOCK_SIZE) - camera.x,
            (block_y_global * BLOCK_SIZE) - camera.y,
            BLOCK_SIZE,
            BLOCK_SIZE
        };

               SDL_Rect DEBUG_CHUNK {
            (chunks[chunk_index].x_off_w) - camera.x, 
            (chunks[chunk_index].y_off_w) - camera.y, 
            CHUNK_RES, 
            CHUNK_RES
        };

        */

        
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
            cam_vel_x = -25;
        if(keystate[SDL_SCANCODE_RIGHT] == 1)
            cam_vel_x = 25;
        if(keystate[SDL_SCANCODE_UP] == 1)
            cam_vel_y = -25;
        if(keystate[SDL_SCANCODE_DOWN] == 1)
            cam_vel_y = 25;
        
        if(camera.x + cam_vel_x >= 0 && camera.x + cam_vel_x <= WORLD_CHUNK_W * CHUNK_SIZE * BLOCK_SIZE)
            camera.x += cam_vel_x;
        if(camera.y + cam_vel_y <= WORLD_CHUNK_H * CHUNK_SIZE * BLOCK_SIZE - SCREEN_HEIGHT 
                && camera.y + cam_vel_y >= 0)
            camera.y += cam_vel_y;

        SDL_SetRenderDrawColor(renderer, 0,0,0,255);
        SDL_RenderClear(renderer);

        SDL_PixelFormat* pixel_format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);

        for(int k = 0; k < chunks.size(); k++) 
        { 
            SDL_Rect chunkrect {
                chunks[k].x_off_w, 
                chunks[k].y_off_w, 
                CHUNK_SIZE * BLOCK_SIZE * 0.95, 
                CHUNK_SIZE * BLOCK_SIZE * 0.95
            };

            SDL_Rect inter;

            if(SDL_HasIntersection(&chunkrect, &camera))
            { 
                for(int i = 0; i < CHUNK_SIZE; i++) 
                {
                    for(int j = 0; j < CHUNK_SIZE; j++) 
                    {
                        Uint32 pixel = chunks[k].arr[i][j].color;
                        Uint8 r,g,b,a;
                        r = 0; g = 0; b = 0; a = 0;
                        
                        SDL_GetRGBA(pixel, pixel_format, &r, &g, &b, &a);

                        SDL_SetRenderDrawColor(renderer, r, g,b, a);
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

        /*
        SDL_SetRenderDrawColor(renderer, 50, 50, 200, 255);
        SDL_RenderDrawRect(renderer, &DEBUG_CHUNK);
        SDL_RenderDrawRect(renderer, &player_b_col);
        SDL_RenderDrawRect(renderer, &DEBUG_BLOCK);
        SDL_RenderFillRect(renderer, &DEBUG_MOUSE);
        SDL_RenderDrawRect(renderer, &bottom_rect);
        */

        SDL_RenderPresent(renderer);
        
        Uint64 end = SDL_GetPerformanceCounter();

        float elapsedMS = (end - start) / (float)SDL_GetPerformanceFrequency() * 1000.0f;
	    SDL_Delay(floor(16.666f - elapsedMS));
    }


    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
