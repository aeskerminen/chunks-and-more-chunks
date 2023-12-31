#ifndef TILE
#define TILE

enum Collider {none=0, soft, hard};
enum TType {air=0, grass, dirt, rock};

const char* const ColliderStrings[] = 
{
    "none",
    "soft",
    "hard"
};

const char* const TTypeStrings[] = 
{
    "air",
    "grass",
    "dirt",
    "rock",
};

typedef struct tile 
{
    TType type;
    Uint32 color;
    Collider col;
} tile;


#endif
