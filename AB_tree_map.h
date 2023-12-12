#ifndef AB_TREE_MAP_H_INCLUDED
#define AB_TREE_MAP_H_INCLUDED
#include "KV_map.h"


typedef struct AB_Tree_Map AB_Tree_Map;

struct AB_Tree_Map
{
    KV_map map;
    struct AB_Tree *TreeConfig;
};

KV_map *AB_tree_Map_init(short a, short b, size_t key_size, size_t value_size);



#endif // AB_TREE_H_INCLUDED
