#ifndef PLAYER
#define PLAYER

#include <SDL2/SDL.h>
#include <vector>

#include "globals.h"
#include "tile.h"
#include "chunk.h"

constexpr int TERMINAL_VELOCITY = 3.81;
const int JUMP_FORCE = 12;


typedef struct player 
{
    float x, y;
    float velx, vely;
    bool jump;
    bool coll[4];
} player;

void do_player_collision(player& player, const std::vector<chunk>& chunks, const SDL_FRect& camera);
void do_player_move(player& player, const Uint8* keystate, const float dt);

#endif
