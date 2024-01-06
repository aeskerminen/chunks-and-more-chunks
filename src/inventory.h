#ifndef INV
#define INV

#include "item.h"

typedef struct inventory 
{
    std::vector<item> contents;
    size_t max_size;
} inventory;

void init_inventory();

#endif
