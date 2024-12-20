#ifndef PLAYER
#define PLAYER

#include <SDL2/SDL.h>
#include <vector>
#include <algorithm>

#include "globals.h"
#include "tile.h"
#include "chunk.h"
#include "inventory.h"


enum HandMode {PLACE=0, ATTACK, NONE};

typedef struct player 
{
    float x, y;
    float velx, vely;
    bool jump;
    bool coll[4];
    inventory inv;
    item* curItem;
    HandMode handmode;
} player;

#endif
