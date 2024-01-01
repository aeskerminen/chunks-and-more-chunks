#ifndef CHUNK
#define CHUNK

#include "tile.h"
#include "globals.h"

typedef struct chunk 
{
    tile arr[CHUNK_SIZE][CHUNK_SIZE];
    int x_off_w;
    int y_off_w;
    bool change;
} chunk;

#endif
