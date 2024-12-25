#include "pointer.h"

int get_mouse_chunk_index()
{
    // Get mouse localtion in world coordinates
    int mx, my;
    SDL_GetMouseState(&mx, &my);

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

std::pair<int, int> get_block_local_indices()
{
    // Get mouse localtion in world coordinates
    int mx, my;
    SDL_GetMouseState(&mx, &my);

    int mx_world = mx + camera.x;
    int my_world = my + camera.y;

    // Get block at world coordinates
    int m_block_global_x = (mx_world / BLOCK_SIZE);
    int m_block_global_y = (my_world / BLOCK_SIZE);

    int m_block_local_x = m_block_global_x % CHUNK_SIZE;
    int m_block_local_y = m_block_global_y % CHUNK_SIZE;

    return std::pair<int, int>{m_block_local_x, m_block_local_y};
}

tile *get_block_at_cursor(std::vector<chunk> &chunks)
{
    int m_block_chunk_index = get_mouse_chunk_index();

    std::pair<int, int> m_block_local_indices = get_block_local_indices();

    int m_block_local_x = std::get<0>(m_block_local_indices);
    int m_block_local_y = std::get<1>(m_block_local_indices);

    // Reference to block
    auto &block = chunks[m_block_chunk_index].arr[m_block_local_x][m_block_local_y];

    // Remove block
    SDL_Log("Type: %s, Color: %d, Collider: %s\n",
            TTypeStrings[block.type], block.color, ColliderStrings[block.col]);

    return &chunks[m_block_chunk_index].arr[m_block_local_x][m_block_local_y];
}

void do_show_mouse_helper(const std::vector<chunk> &chunks, SDL_Renderer *renderer)
{
    // Get mouse localtion in world coordinates
    int mx, my;
    SDL_GetMouseState(&mx, &my);

    int mx_world = mx + camera.x;
    int my_world = my + camera.y;

    // Get block at world coordinates
    int m_block_global_x = (mx_world / BLOCK_SIZE);
    int m_block_global_y = (my_world / BLOCK_SIZE);

    int m_block_local_x = m_block_global_x % CHUNK_SIZE;
    int m_block_local_y = m_block_global_y % CHUNK_SIZE;

    // Get chunk from global block position
    int m_block_chunk_index_x = ceil(m_block_global_x / CHUNK_SIZE);
    int m_block_chunk_index_y = ceil(m_block_global_y / CHUNK_SIZE);

    // Get actual index
    int m_block_chunk_index = WORLD_CHUNK_W * m_block_chunk_index_y + m_block_chunk_index_x;

    // Reference to block
    auto &block = chunks[m_block_chunk_index].arr[m_block_local_x][m_block_local_y];

    if (block.col != Collider::none)
    {
        SDL_FRect helper{
            m_block_global_x * BLOCK_SIZE - camera.x,
            m_block_global_y * BLOCK_SIZE - camera.y,
            BLOCK_SIZE,
            BLOCK_SIZE};

        SDL_SetRenderDrawColor(renderer, 200, 100, 200, 100);
        SDL_RenderDrawRectF(renderer, &helper);
    }
}
