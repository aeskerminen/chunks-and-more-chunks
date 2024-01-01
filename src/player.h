#ifndef PLAYER
#define PLAYER

constexpr int TERMINAL_VELOCITY = 9.81 * 5;
const int JUMP_FORCE = 50;

typedef struct player 
{
    float x, y;
    float velx, vely;
    bool jump;
    bool coll[4];
} player;

#endif
