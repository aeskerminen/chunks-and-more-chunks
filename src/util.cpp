#include "util.h"

int get_location_chunk_index(int mx, int my)
{
    int mx_world = mx + camera.x;
    int my_world = my + camera.y;

    // Get block at world coordinates
    int m_block_global_x = (mx_world / BLOCK_SIZE);
    int m_block_global_y = (my_world / BLOCK_SIZE);

    // Get chunk from global block position
    int m_block_chunk_index_x = ceil(m_block_global_x / CHUNK_SIZE);
    int m_block_chunk_index_y = ceil(m_block_global_y / CHUNK_SIZE);

    // Get actual index
    int m_block_chunk_index = WORLD_CHUNK_W * m_block_chunk_index_y + m_block_chunk_index_x;

    return m_block_chunk_index;
}

std::pair<int, int> get_block_local_indices(int mx, int my)
{
    int mx_world = mx + camera.x;
    int my_world = my + camera.y;

    // Get block at world coordinates
    int m_block_global_x = (mx_world / BLOCK_SIZE);
    int m_block_global_y = (my_world / BLOCK_SIZE);

    int m_block_local_x = m_block_global_x % CHUNK_SIZE;
    int m_block_local_y = m_block_global_y % CHUNK_SIZE;

    return std::pair<int, int>{m_block_local_x, m_block_local_y};
}