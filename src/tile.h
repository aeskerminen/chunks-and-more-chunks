#ifndef TILE
#define TILE

#include <stdint.h>

enum Collider
{
    none = 0,
    soft,
    hard
};
enum TType
{
    air = 0,
    grass,
    dirt,
    rock
};

const char *const ColliderStrings[] =
    {
        "none",
        "soft",
        "hard"};

const char *const TTypeStrings[] =
    {
        "air",
        "grass",
        "dirt",
        "rock",
};

typedef struct tile
{
    TType type;
    uint32_t color;
    Collider col;
} tile;

#endif
