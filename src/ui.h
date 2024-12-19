#ifndef UI_H
#define UI_H

#include <stdio.h>

#include "tile.h"
#include "player.h"

#define NK_MEMSET memset
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT

#include "../lib/nuklear.h"
#include "../lib/nuklear_sdl_renderer.h"

extern struct nk_context *ctx;

void render_inventory(const player &player);
void init_ui(SDL_Window *window, SDL_Renderer *renderer);

#endif