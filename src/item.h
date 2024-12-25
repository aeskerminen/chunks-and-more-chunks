#ifndef ITEM
#define ITEM

typedef struct Item
{
    int item_id;

    float x, y;

    bool stackable;
    int count;
} item;

#endif
