#include "player.h"

void do_player_collision(player& player, const std::vector<chunk>& chunks, const SDL_FRect& camera) 
{
        bool& col_l = player.coll[0];
        bool& col_b = player.coll[1];
        bool& col_r = player.coll[2];
        bool& col_t = player.coll[3];
        
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
        SDL_FRect player_col {(player.x) - camera.x, (player.y) - camera.y, PLAYER_WIDTH, PLAYER_HEIGHT};
        
        // THE BLOCK ON WHICH THE PLAYERS HEAD IS ON
        SDL_Point PHeadBlock {floor(player.x / BLOCK_SIZE), floor(player.y / BLOCK_SIZE)};

        // BOTTOM & TOP COLLISION        
        {

            // BOTTOM
            for(int k = 0; k < PLAYER_W_MULT * 2; k++) 
            {
                int actual_loc = k - 1;
                
                SDL_Point globalCoord {PHeadBlock.x + actual_loc, PHeadBlock.y + PLAYER_H_MULT};
                SDL_Point chunkCoord = CIndexFromBlock(globalCoord.x, globalCoord.y);
                
                int actualChunk = WORLD_CHUNK_W * chunkCoord.y + chunkCoord.x;
                SDL_Point localCoord = GetLocalBlockPoint(globalCoord.x, globalCoord.y);
                
                SDL_FRect collider {
                    globalCoord.x * BLOCK_SIZE - camera.x,
                    globalCoord.y * BLOCK_SIZE - camera.y,
                    BLOCK_SIZE,
                    BLOCK_SIZE
                };
                
                if(SDL_HasIntersectionF(&collider, &player_col) && 
                    chunks[actualChunk].arr[localCoord.x][localCoord.y].col 
                    == Collider::hard)
                    col_b = true;

                //SDL_RenderDrawRectF(renderer, &collider);
            }

            // TOP
            for(int k = 0; k < PLAYER_W_MULT * 2; k++) 
            {
                int actual_loc = k - 1;
                
                SDL_Point globalCoord {PHeadBlock.x + actual_loc, PHeadBlock.y - 1};
                SDL_Point chunkCoord = CIndexFromBlock(globalCoord.x, globalCoord.y);
                
                int actualChunk = WORLD_CHUNK_W * chunkCoord.y + chunkCoord.x;
                SDL_Point localCoord = GetLocalBlockPoint(globalCoord.x, globalCoord.y);
                
                SDL_FRect collider {
                    globalCoord.x * BLOCK_SIZE - camera.x,
                    globalCoord.y * BLOCK_SIZE - camera.y,
                    BLOCK_SIZE,
                    BLOCK_SIZE * 1.025f
                };
                
                if(SDL_HasIntersectionF(&collider, &player_col) && 
                    chunks[actualChunk].arr[localCoord.x][localCoord.y].col 
                    == Collider::hard)
                    col_t = true;

              //  SDL_RenderDrawRectF(renderer, &collider);
            }


        }

        // LEFT & RIGHT COLLISION
        {

            // LEFT
            for(int k = 0; k < PLAYER_H_MULT; k++) 
            {        
                SDL_Point globalCoord {PHeadBlock.x - 1, PHeadBlock.y + k};
                SDL_Point chunkCoord = CIndexFromBlock(globalCoord.x, globalCoord.y);
                
                int actualChunk = WORLD_CHUNK_W * chunkCoord.y + chunkCoord.x;
                SDL_Point localCoord = GetLocalBlockPoint(globalCoord.x, globalCoord.y);
                
                SDL_FRect collider {
                    globalCoord.x * BLOCK_SIZE - camera.x,
                    globalCoord.y * BLOCK_SIZE - camera.y,
                    BLOCK_SIZE * 1.025f,
                    BLOCK_SIZE
                };
                
                if(SDL_HasIntersectionF(&collider, &player_col) && 
                    chunks[actualChunk].arr[localCoord.x][localCoord.y].col 
                    == Collider::hard)
                    col_l = true;

              //  SDL_RenderDrawRectF(renderer, &collider);
            }

            // RIGHT
            for(int k = 0; k < PLAYER_H_MULT; k++) 
            {        
                SDL_Point globalCoord {PHeadBlock.x + PLAYER_W_MULT, PHeadBlock.y + k};
                SDL_Point chunkCoord = CIndexFromBlock(globalCoord.x, globalCoord.y);
                
                int actualChunk = WORLD_CHUNK_W * chunkCoord.y + chunkCoord.x;
                SDL_Point localCoord = GetLocalBlockPoint(globalCoord.x, globalCoord.y);
                
                SDL_FRect collider {
                    globalCoord.x * BLOCK_SIZE - camera.x,
                    globalCoord.y * BLOCK_SIZE - camera.y,
                    BLOCK_SIZE * 1.025f,
                    BLOCK_SIZE
                };
                
                if(SDL_HasIntersectionF(&collider, &player_col) && 
                    chunks[actualChunk].arr[localCoord.x][localCoord.y].col 
                    == Collider::hard)
                    col_r = true;

              ////  SDL_RenderDrawRectF(renderer, &collider);
            }
        }
}

void do_player_move(player& player, const Uint8* keystate, const float dt) 
{
        bool& lcol = player.coll[0];
        bool& bcol = player.coll[1];
        bool& rcol = player.coll[2];
        bool& tcol = player.coll[3];

        // PLAYER
        if(keystate[SDL_SCANCODE_A] == 1)
            player.velx = -0.45;
        if(keystate[SDL_SCANCODE_D] == 1)
            player.velx = 0.45;
        if(keystate[SDL_SCANCODE_S] == 1 && bcol && !tcol && !player.jump) 
        {
            player.vely -= JUMP_FORCE;
            player.jump = true;
        }
        if (keystate[SDL_SCANCODE_S] == 0) { player.jump = false; }
    

        int gmult;

        if(bcol)
            gmult = 0;
        else
            gmult = 1;

        player.vely += GRAVITY * gmult;
        
        if(player.vely > TERMINAL_VELOCITY)
            player.vely = TERMINAL_VELOCITY;
        if(tcol)
            player.vely = GRAVITY;

        // Add motion to player
        if(!bcol)
            player.y += player.vely * dt;
        else if(bcol && player.vely < 0)
            player.y += player.vely * dt;

        if(!lcol && player.velx < 0)
            player.x += player.velx * dt;
        if(!rcol && player.velx > 0)
            player.x += player.velx * dt;

        player.velx = 0; 
}

