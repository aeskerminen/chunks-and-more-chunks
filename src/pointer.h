#ifndef POINTER
#define POINTER

#include <SDL2/SDL.h>
#include <vector>
#include <cmath>
#include "camera.h"
#include "chunk.h"

tile *get_block_at_cursor(std::vector<chunk> &chunks);
void do_show_mouse_helper(const std::vector<chunk> &chunks, SDL_Renderer *renderer);

#endif