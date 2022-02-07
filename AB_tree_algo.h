#ifndef AB_TREE_ALGO_H_INCLUDED
#define AB_TREE_ALGO_H_INCLUDED

typedef struct KV_pair KV_pair;
typedef struct AB_internel_node AB_internel_node;
typedef struct AB_Tree AB_Tree;
static struct KV_pair
{
    void *key, *value;
};

static struct AB_internel_node
{
    short KeyCount;
    char IsBottom ;

};

struct AB_Tree
{
    struct AB_internel_node *root;
    struct AB_internel_node *SearchPath[32];
    short SearchBranch[32];
    void *privae_key, *privae_value;
    int (*KeyComp)(const void*, const void*);
    void (*KeyDestroy)(const void*);
    void (*ValueDestroy)(const void*);
    size_t key_size, value_size;
    int SearchPathLength ;
    short a, b;
    int KeyIsFound;

};
static inline KV_pair KV(void *k, void *v);
static inline AB_internel_node *MakeNode(AB_Tree *self, size_t child_size, int IsBottom);
void AB_tree_Insert_algo(AB_Tree *self, KV_pair kv);
int AB_tree_Search_algo(AB_Tree *self, void *key);

static inline AB_internel_node *MakeNode(AB_Tree *self, size_t child_size, int IsBottom)
{
    AB_internel_node *new_node;
    size_t kv_size = sizeof(AB_internel_node) + self->key_size*(self->b-1) + child_size*(self->b);
    new_node = (AB_internel_node*)malloc(kv_size);
    new_node->IsBottom = IsBottom;
    new_node->KeyCount = 0;
    return new_node;
}

static inline KV_pair KV(void *k, void *v)
{
    KV_pair ret = {.key = k, .value = v};
    return ret;
}
#endif // AB_TREE_ALGO_H_INCLUDED
