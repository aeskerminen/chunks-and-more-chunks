#ifndef CHUNK
#define CHUNK

#include "tile.h"

const int CHUNK_SIZE = 16;

typedef struct chunk 
{
    tile arr[CHUNK_SIZE][CHUNK_SIZE];
    int x_off_w;
    int y_off_w;
    bool change;
} chunk;

#endif
