#include "item.h"

bool check_collision(SDL_Rect a, SDL_Rect b)
{
    // The sides of the rectangles
    int leftA, leftB;
    int rightA, rightB;
    int topA, topB;
    int bottomA, bottomB;

    // Calculate the sides of rect A
    leftA = a.x;
    rightA = a.x + a.w;
    topA = a.y;
    bottomA = a.y + a.h;

    // Calculate the sides of rect B
    leftB = b.x;
    rightB = b.x + b.w;
    topB = b.y;
    bottomB = b.y + b.h;

    if (bottomA <= topB)
    {
        return false;
    }

    if (topA >= bottomB)
    {
        return false;
    }

    if (rightA <= leftB)
    {
        return false;
    }

    if (leftA >= rightB)
    {
        return false;
    }

    return true;
}

bool check_item_collision(const item &item, const std::vector<chunk> &chunks)
{
    const int x = item.x, y = item.y;

    SDL_Rect item_rect{x - camera.x, y - camera.y, BLOCK_SIZE, BLOCK_SIZE};

    auto CIndexFromBlock = [](int bx, int by)
    {
        SDL_Point point;
        point.x = ceil(bx / CHUNK_SIZE);
        point.y = ceil(by / CHUNK_SIZE);

        return point;
    };

    auto GetLocalBlockPoint = [](int bx, int by)
    {
        SDL_Point point;
        point.x = floor(bx % CHUNK_SIZE);
        point.y = floor(by % CHUNK_SIZE);

        return point;
    };

    {
        // Get block at world coordinates
        int m_block_global_x = (x / BLOCK_SIZE);
        int m_block_global_y = ((y + BLOCK_SIZE) / BLOCK_SIZE);

        SDL_Point chunkCoord = CIndexFromBlock(m_block_global_x, m_block_global_y);

        int actualChunk = WORLD_CHUNK_W * chunkCoord.y + chunkCoord.x;
        SDL_Point localCoord = GetLocalBlockPoint(m_block_global_x, m_block_global_y);

        SDL_Rect test_bottom_m{m_block_global_x * BLOCK_SIZE - camera.x, m_block_global_y * BLOCK_SIZE - camera.y, BLOCK_SIZE, BLOCK_SIZE};

        bool bottom_m_collideable = chunks[actualChunk].arr[localCoord.x][localCoord.y].col == Collider::hard;

        bool collided = (check_collision(item_rect, test_bottom_m) && bottom_m_collideable);

        return collided;
    }
}

void do_tick_items(std::vector<item> &items, const std::vector<chunk> &chunks, double dt)
{
    for (item &i : items)
    {
        SDL_Rect item_rect = SDL_Rect{i.x - camera.x, i.y - camera.y, ITEM_SIZE, ITEM_SIZE};
        if (!check_item_collision(i, chunks))
        {
            i.y += GRAVITY * 0.5f * dt;
        }
    }
}
