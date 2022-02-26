#ifndef AB_TREE_MAP_H_INCLUDED
#define AB_TREE_MAP_H_INCLUDED

typedef struct AB_Tree_Map AB_Tree_Map;

struct AB_Tree_Map
{
    int (*Insert)(AB_Tree_Map *self, const void *key, const void *value);
    int (*Delete)(AB_Tree_Map *self, const void *key);
    int (*Search)(AB_Tree_Map *self, const void *key);
    void (*SetKeyComp)(AB_Tree_Map *self, int (*KeyComp)(const void*,const void*));
    void (*SetKeyDestroy)(AB_Tree_Map *self, void(*KeyDestroy)(const void*));
    void (*SetValueDestroy)(AB_Tree_Map *self, void(*ValueDestroy)(const void*));
    void (*SetPrintData)(AB_Tree_Map *self, const void*, const void*);
    void (*GetValue)(AB_Tree_Map *self, void *content);

    struct AB_Tree *TreeConfig;
};

void AB_tree_Map_init(struct AB_Tree_Map* self, short a, short b, size_t key_size, size_t value_size);



#endif // AB_TREE_H_INCLUDED
