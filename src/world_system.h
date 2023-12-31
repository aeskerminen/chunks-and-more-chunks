#ifndef WORLD_SYSTEM
#define WORLD_SYSTEM

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <vector>

#include "tile.h"
#include "chunk.h"

const int BLOCK_SIZE = 35;

const int WORLD_CHUNK_W = 16;
const int WORLD_CHUNK_H = 16;

constexpr int CHUNK_PIXEL_WIDTH = BLOCK_SIZE * CHUNK_SIZE;
constexpr int WOLRD_RES = WORLD_CHUNK_W * WORLD_CHUNK_H * CHUNK_PIXEL_WIDTH * CHUNK_PIXEL_WIDTH;

std::vector<chunk> generate_world(int w_width, int w_height);
void SAVE_WORLD(std::vector<chunk>& chunks);
void LOAD_WORLD();

#endif
