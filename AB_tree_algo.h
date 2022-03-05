#ifndef AB_TREE_ALGO_H_INCLUDED
#define AB_TREE_ALGO_H_INCLUDED
#include<stdlib.h>
#include<string.h>
typedef struct KV_pair KV_pair;
typedef struct AB_node AB_node;
typedef struct AB_Tree AB_Tree;
struct KV_pair
{
    const void *key, *value;
};

struct AB_node
{
    short KeyCount;
    char IsBottom ;

};

struct AB_Tree
{
    struct AB_node *root;
    struct AB_node *SearchPath[32];
    short SearchBranch[32];
    void *privae_value;
    int (*KeyComp)(const void*, const void*);
    void (*KeyDestroy)(const void*);
    void (*ValueDestroy)(const void*);
    size_t key_size, value_size;
    int SearchPathLength ;
    short a, b;
    int KeyIsFound;

};
static inline KV_pair KV(const void *k, const void *v);
//static inline AB_node *MakeNode(AB_Tree *self, size_t child_size, int IsBottom);
void AB_tree_Insert_algo(AB_Tree *self, KV_pair kv);
int AB_tree_Search_algo(AB_Tree *self, const void *key);
int AB_tree_Delete_algo(AB_Tree *self, const void *key);

static inline KV_pair KV(const void *k, const void *v)
{
    KV_pair ret = {.key = k, .value = v};
    return ret;
}
#endif // AB_TREE_ALGO_H_INCLUDED
