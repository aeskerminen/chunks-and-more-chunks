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

void do_player_move(player &player, const Uint8 *keystate, const float dt)
{
    bool &lcol = player.coll[0];
    bool &bcol = player.coll[1];
    bool &rcol = player.coll[2];
    bool &tcol = player.coll[3];

    int right = 0, left = 0;

    // PLAYER
    if (keystate[SDL_SCANCODE_A] == 1)
        left = 1;
    else
    {
        left = 0;
    }
    if (keystate[SDL_SCANCODE_D] == 1)
        right = 1;
    else
    {
        right = 0;
    }

    if (keystate[SDL_SCANCODE_S] == 1)
    {
        player.jump = true;
    }
    else
    {
        player.jump = false;
    }

    float speedY = 0.0f;
    float speedX = 0.0f;

    float speedMX = 4.0f;
    float speedMY = -7.0f;

    float friction = 0.95f;
    float runSpeed = 0.3f;

    float jumpForce = 10.0f;

    speedX += ((right * runSpeed) - (left * runSpeed)) * dt;

    if (speedX > speedMX)
    {
        speedMX = speedMX;
    }
    if (-speedX > speedMX)
    {
        speedX = -speedMX;
    }

    player.x += speedX * friction;

    speedY -= ((bcol ? 1.0f : 0) * (player.jump ? 1.0f : 0) * jumpForce) - GRAVITY;

    if (speedY < speedMY)
    {
        speedY = speedMY;
    }

    if(bcol && speedY > 0) {
        speedY = 0;
    }

    player.y += speedY;

    if (bcol)
    {
        speedY = 0;
        player.jump = false;
    }
}
