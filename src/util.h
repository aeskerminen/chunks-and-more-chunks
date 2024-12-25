#ifndef UTIL_H
#define UTIL_H

#include "camera.h"
#include <cmath>

std::pair<int, int> get_block_local_indices(int mx, int my);
int get_location_chunk_index(int mx, int my);

#endif