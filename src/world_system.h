#ifndef WORLD_SYSTEM
#define WORLD_SYSTEM

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <vector>

#include "tile.h"
#include "chunk.h"
#include "globals.h"

std::vector<chunk> generate_world(int w_width, int w_height);
void SAVE_WORLD(std::vector<chunk>& chunks);
void LOAD_WORLD();

#endif
