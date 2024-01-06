#ifndef GLOBALS
#define GLOBALS

const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1200;

const float GRAVITY = 0.11;

const int BLOCK_SIZE = 35;
const int CHUNK_SIZE = 16;

const int WORLD_CHUNK_W = 16;
const int WORLD_CHUNK_H = 16;

constexpr int CHUNK_PIXEL_WIDTH = BLOCK_SIZE * CHUNK_SIZE;
constexpr int WOLRD_RES = WORLD_CHUNK_W * WORLD_CHUNK_H * CHUNK_PIXEL_WIDTH * CHUNK_PIXEL_WIDTH;

const int PLAYER_W_MULT = 2;
const int PLAYER_H_MULT = 3;

constexpr int PLAYER_WIDTH = BLOCK_SIZE*PLAYER_W_MULT;
constexpr int PLAYER_HEIGHT = BLOCK_SIZE*PLAYER_H_MULT;

#endif
