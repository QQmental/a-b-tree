#ifndef KV_MAP_H
#define KV_MAP_H

typedef struct KV_map KV_map;

struct KV_map
{
    int (*Insert)(KV_map *self, const void *key, const void *value);
    int (*Delete)(KV_map *self, const void *key);
    int (*Search)(KV_map *self, const void *key);
    void (*SetKeyComp)(KV_map *self, int (*KeyComp)(const void*,const void*));
    void (*SetKeyDestroy)(KV_map *self, void(*KeyDestroy)(const void*));
    void (*SetValueDestroy)(KV_map *self, void(*ValueDestroy)(const void*));
    void (*GetValue)(KV_map *self, void *content);  
};


#endif