#include "world_system.h"

std::vector<chunk> generate_world(int w_width, int w_height) 
{
    auto noise = IMG_Load("assets/samplenoise.png");
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
        c.x_off_w = CHUNK_PIXEL_WIDTH * (k % w_width);
        c.y_off_w = CHUNK_PIXEL_WIDTH * height;
    
        chunks.push_back(c);

        if((k+1) % w_height == 0 && k != 0)
            height++;
    }

    for(int y = 0; y < CHUNK_SIZE * WORLD_CHUNK_H; y++) 
    {
        for(int x = 0; x < CHUNK_SIZE * WORLD_CHUNK_W; x++) 
        {
            int cindex = WORLD_CHUNK_W * ceil(y / CHUNK_SIZE) + ceil(x / CHUNK_SIZE);
            tile* curtile = &chunks[cindex].arr[x % CHUNK_SIZE][y % CHUNK_SIZE];
            
            int texture_x = x % noise_w;
            int texture_y = y % noise_h;

            Uint8 r, g, b, a;
            SDL_GetRGBA(textureBuffer[texture_x][texture_y], formatPix, &r, &g, &b, &a);
            Uint8 luminance = (0.2126*r + 0.7152*g + 0.0722*b);

            curtile->color = textureBuffer[texture_x][texture_y];
            
            if(luminance > (255 / 2))
                curtile->col = Collider::hard;
            else
                curtile->col = Collider::none;

            if(luminance < (255/2))
                curtile->type = TType::air;
            else if(luminance < 170)
                curtile->type = TType::grass;
            else if(luminance < 213)
                curtile->type = TType::dirt;
            else
                curtile->type = TType::rock;
        }
    }
 
    return chunks;
}

void SAVE_WORLD(std::vector<chunk>& chunks) 
{
    
}

void LOAD_WORLD() 
{

}
