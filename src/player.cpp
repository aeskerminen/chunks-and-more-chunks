
#include "player.h"
#include <cmath>

void do_player_collision(player &player, const std::vector<chunk> &chunks, const SDL_FRect &camera)
{
    bool &col_l = player.coll[0];
    bool &col_b = player.coll[1];
    bool &col_r = player.coll[2];
    bool &col_t = player.coll[3];

    col_l = col_b = col_r = col_t = false;

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

    // PLAYER COLLIDER
    SDL_FRect player_col{(player.x) - camera.x, (player.y) - camera.y, PLAYER_WIDTH, PLAYER_HEIGHT};
    SDL_FRect player_col_vert{(player.x) - camera.x + (PLAYER_WIDTH * 0.025f) / 2, (player.y) - camera.y, PLAYER_WIDTH * 0.975f, PLAYER_HEIGHT};

    // THE BLOCK ON WHICH THE PLAYERS HEAD IS ON
    SDL_Point PHeadBlock{floor(player.x / BLOCK_SIZE), floor(player.y / BLOCK_SIZE)};

    // BOTTOM & TOP COLLISION
    {

        // BOTTOM
        for (int k = 0; k < PLAYER_W_MULT * 2; k++)
        {
            int actual_loc = k - 1;

            SDL_Point globalCoord{PHeadBlock.x + actual_loc, PHeadBlock.y + PLAYER_H_MULT};
            SDL_Point chunkCoord = CIndexFromBlock(globalCoord.x, globalCoord.y);

            int actualChunk = WORLD_CHUNK_W * chunkCoord.y + chunkCoord.x;
            SDL_Point localCoord = GetLocalBlockPoint(globalCoord.x, globalCoord.y);

            SDL_FRect collider{
                globalCoord.x * BLOCK_SIZE - camera.x,
                globalCoord.y * BLOCK_SIZE - camera.y,
                BLOCK_SIZE,
                BLOCK_SIZE};

            if (SDL_HasIntersectionF(&collider, &player_col_vert) &&
                chunks[actualChunk].arr[localCoord.x][localCoord.y].col == Collider::hard)
                col_b = true;

            // SDL_RenderDrawRectF(renderer, &collider);
        }

        // TOP
        for (int k = 0; k < PLAYER_W_MULT * 2; k++)
        {
            int actual_loc = k - 1;

            SDL_Point globalCoord{PHeadBlock.x + actual_loc, PHeadBlock.y - 1};
            SDL_Point chunkCoord = CIndexFromBlock(globalCoord.x, globalCoord.y);

            int actualChunk = WORLD_CHUNK_W * chunkCoord.y + chunkCoord.x;
            SDL_Point localCoord = GetLocalBlockPoint(globalCoord.x, globalCoord.y);

            SDL_FRect collider{
                globalCoord.x * BLOCK_SIZE - camera.x,
                globalCoord.y * BLOCK_SIZE - camera.y,
                BLOCK_SIZE,
                BLOCK_SIZE * 1.025f};

            if (SDL_HasIntersectionF(&collider, &player_col) &&
                chunks[actualChunk].arr[localCoord.x][localCoord.y].col == Collider::hard)
                col_t = true;

            //  SDL_RenderDrawRectF(renderer, &collider);
        }
    }

    // LEFT & RIGHT COLLISION
    {

        // LEFT
        for (int k = 0; k < PLAYER_H_MULT; k++)
        {
            SDL_Point globalCoord{PHeadBlock.x - 1, PHeadBlock.y + k};
            SDL_Point chunkCoord = CIndexFromBlock(globalCoord.x, globalCoord.y);

            int actualChunk = WORLD_CHUNK_W * chunkCoord.y + chunkCoord.x;
            SDL_Point localCoord = GetLocalBlockPoint(globalCoord.x, globalCoord.y);

            SDL_FRect collider{
                globalCoord.x * BLOCK_SIZE - camera.x,
                globalCoord.y * BLOCK_SIZE - camera.y,
                BLOCK_SIZE * 1.025f,
                BLOCK_SIZE};

            if (SDL_HasIntersectionF(&collider, &player_col) &&
                chunks[actualChunk].arr[localCoord.x][localCoord.y].col == Collider::hard)
                col_l = true;

            //  SDL_RenderDrawRectF(renderer, &collider);
        }

        // RIGHT
        for (int k = 0; k < PLAYER_H_MULT; k++)
        {
            SDL_Point globalCoord{PHeadBlock.x + PLAYER_W_MULT, PHeadBlock.y + k};
            SDL_Point chunkCoord = CIndexFromBlock(globalCoord.x, globalCoord.y);

            int actualChunk = WORLD_CHUNK_W * chunkCoord.y + chunkCoord.x;
            SDL_Point localCoord = GetLocalBlockPoint(globalCoord.x, globalCoord.y);

            SDL_FRect collider{
                globalCoord.x * BLOCK_SIZE - camera.x,
                globalCoord.y * BLOCK_SIZE - camera.y,
                BLOCK_SIZE * 1.025f,
                BLOCK_SIZE};

            if (SDL_HasIntersectionF(&collider, &player_col) &&
                chunks[actualChunk].arr[localCoord.x][localCoord.y].col == Collider::hard)
                col_r = true;

            ////  SDL_RenderDrawRectF(renderer, &collider);
        }
    }
}
