#ifndef ITEM
#define ITEM

#include <SDL2/SDL.h>

#include <vector>
#include <cmath>

#include "globals.h"
#include "chunk.h"
#include "camera.h"

typedef struct Item
{
    int item_id;

    float x, y;

    bool stackable;
    int count;
} item;

bool check_item_collision(const item &item, const std::vector<chunk> &chunks);
void do_tick_items(std::vector<item> &items, const std::vector<chunk> &chunks, double dt);

#endif
