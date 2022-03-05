#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include"AB_tree_map.h"
#include"AB_tree_algo.h"
#include"AB_node_acc_mod.h"

typedef struct KV_pair KV_pair;
typedef struct AB_Tree AB_Tree;
typedef struct AB_Tree_Map AB_Tree_Map;
/*
struct AB_Tree_Map
{
    int (*Insert)(AB_Tree_Map *self, const void *key, const void *value);
    int (*Delete)(AB_Tree_Map *self, const void *key);
    int (*Search)(AB_Tree_Map *self, const void *key);
    void (*SetKeyComp)(AB_Tree_Map *self, void (*KeyComp)(const void*,const void*));
    void (*SetKeyDestroy)(AB_Tree_Map *self, void(*KeyDestroy)(const void*));
    void (*SetValueDestroy)(AB_Tree_Map *self, void(*ValueDestroy)(const void*));

    void (*PrintData)(const void*, const void*);
    struct AB_Tree *TreeConfig;
};
*/
void AB_tree_Map_init(struct AB_Tree_Map* self, short a, short b, size_t key_size, size_t value_size);
static int Insert(AB_Tree_Map *Tree_Map, const void *key, const void *value);
static int Search(AB_Tree_Map *Tree_Map, const void *key);
static int Delete(AB_Tree_Map *Tree_Map, const void *key);
static void SetKeyComp(AB_Tree_Map *Tree_Map, int (*KeyComp)(const void*,const void*));
static void DoNothing(const void *ptr);
static int Comp_int(const void *a, const void *b);
static void SetKeyDestroy(AB_Tree_Map *Tree_Map, void(*KeyDestroy)(const void *));
static void SetValueDestroy(AB_Tree_Map *Tree_Map, void(*ValueDestroy)(const void *));
static void GetFoundValue(AB_Tree_Map *Tree_Map, void *ValueContainer);
void AB_tree_Map_init(struct AB_Tree_Map* Tree_Map, short a, short b, size_t key_size, size_t value_size)
{
    Tree_Map->Insert = Insert;
    Tree_Map->Delete = Delete;
    Tree_Map->Search = Search;
    Tree_Map->SetKeyComp = SetKeyComp;
    Tree_Map->SetKeyDestroy = SetKeyDestroy;
    Tree_Map->SetValueDestroy = SetValueDestroy;
    Tree_Map->GetValue = GetFoundValue;

    AB_Tree *TreeConfig = (AB_Tree*)malloc(sizeof(AB_Tree));
    TreeConfig->a = a;
    TreeConfig->b = b;
    TreeConfig->key_size = key_size;
    TreeConfig->value_size = value_size;
    TreeConfig->root = RequestAB_node(TreeConfig, 1);
    TreeConfig->root->KeyCount = 0;
    TreeConfig->privae_value = NULL;
    TreeConfig->KeyComp = Comp_int;
    TreeConfig->KeyDestroy = DoNothing;
    TreeConfig->ValueDestroy = DoNothing;

    Tree_Map->TreeConfig = TreeConfig;

}


static int Insert(AB_Tree_Map *Tree_Map, const void *key, const void *value)
{
    AB_Tree *TreeConfig = Tree_Map->TreeConfig;
    AB_tree_Insert_algo(TreeConfig, KV(key, value));

    if(TreeConfig->privae_value)
    {
        //TreeConfig->ValueDestroy(TreeConfig->privae_value);
        //memcpy(TreeConfig->privae_value, value, TreeConfig->value_size);
        return 0;
    }
    return 1 ;
}
static int Search(AB_Tree_Map *Tree_Map, const void *key)
{
    AB_tree_Search_algo(Tree_Map->TreeConfig, key);

    return Tree_Map->TreeConfig->KeyIsFound;
}
static int Delete(AB_Tree_Map *Tree_Map, const void *key)
{
    return AB_tree_Delete_algo(Tree_Map->TreeConfig, key);
}

static void SetKeyComp(AB_Tree_Map* Tree_Map, int (*KeyComp)(const void*,const void*))
{
    Tree_Map->TreeConfig->KeyComp = KeyComp;
}
static void DoNothing(const void *key)
{
    return;
}

static int Comp_int(const void *a, const void *b)
{
    if (*(int*)a > *(int*)b)
        return 1 ;

    if (*(int*)a < *(int*)b)
        return -1 ;

    if (*(int*)a == *(int*)b)
        return 0 ;
}

static void SetKeyDestroy(AB_Tree_Map *Tree_Map, void(*KeyDestroy)(const void *))
{
    Tree_Map->TreeConfig->KeyDestroy = KeyDestroy;
}
static void SetValueDestroy(AB_Tree_Map *Tree_Map, void(*ValueDestroy)(const void *))
{
    Tree_Map->TreeConfig->ValueDestroy = ValueDestroy;
}
static void GetFoundValue(AB_Tree_Map *Tree_Map, void *ValueContainer)
{
    memcpy(ValueContainer, Tree_Map->TreeConfig->privae_value, Tree_Map->TreeConfig->value_size);
}

void clea(AB_Tree_Map *Tree_Map)
{
    //free(Tree_Map->TreeConfig->root);
    
    FreeAB_node(Tree_Map->TreeConfig, Tree_Map->TreeConfig->root);
    free(Tree_Map->TreeConfig);
    //free(Tree_Map->TreeConfig);
}
