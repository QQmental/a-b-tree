#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include"AB_tree.h"
#include"AB_tree_algo.h"


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
void AB_tree_init(struct AB_Tree_Map* self, short a, short b, size_t key_size, size_t value_size);
static int Insert(AB_Tree_Map *self, void *key, void *value);
static int Search(AB_Tree_Map *self, void *key);
static int Delete(AB_Tree_Map *self, void *key);
static void SetKeyComp(AB_Tree_Map *self, void (*KeyComp)(void*,void*));
static void DoNothing(const void *ptr);
static int Comp_int(const void *a, const void *b);
static void SetKeyDestroy(AB_Tree_Map *self, void(*KeyDestroy)(const void *));
static void SetValueDestroy(AB_Tree_Map *self, void(*ValueDestroy)(const void *));
static void GetFoundValue(AB_Tree_Map *self, void *ValueContainer);
void AB_tree_init(struct AB_Tree_Map* self, short a, short b, size_t key_size, size_t value_size)
{
    self->Insert = Insert;
    self->Delete = Delete;
    self->Search = Search;
    self->SetKeyComp = SetKeyComp;
    self->SetKeyDestroy = SetKeyDestroy;
    self->SetValueDestroy = SetValueDestroy;
    self->GetValue = GetFoundValue;

    AB_Tree *TreeConfig = (AB_Tree*)malloc(sizeof(AB_Tree));
    TreeConfig->a = a;
    TreeConfig->b = b;
    TreeConfig->key_size = key_size;
    TreeConfig->value_size = value_size;
    TreeConfig->root = MakeNode(TreeConfig, value_size, 1);
    TreeConfig->root->KeyCount = 0;
    TreeConfig->privae_key = NULL;
    TreeConfig->privae_value = NULL;
    TreeConfig->KeyComp = Comp_int;
    TreeConfig->KeyDestroy = DoNothing;
    TreeConfig->ValueDestroy = DoNothing;

    self->TreeConfig = TreeConfig;

}


static int Insert(AB_Tree_Map *self, void *key, void *value)
{
    AB_Tree *TreeConfig = self->TreeConfig;
    AB_tree_Insert_algo(TreeConfig, KV(key, value));

    if(TreeConfig->privae_value)
    {
        //TreeConfig->ValueDestroy(TreeConfig->privae_value);
        //memcpy(TreeConfig->privae_value, value, TreeConfig->value_size);
        return 0;
    }
    return 1 ;
}
static int Search(AB_Tree_Map *self, void *key)
{
    AB_tree_Search_algo(self->TreeConfig, key);

    return self->TreeConfig->KeyIsFound;
}
static int Delete(AB_Tree_Map *self, void *key)
{
    return 0;
}

static void SetKeyComp(AB_Tree_Map* self, void (*KeyComp)(void*,void*))
{
    self->TreeConfig->KeyComp = KeyComp;
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

static void SetKeyDestroy(AB_Tree_Map *self, void(*KeyDestroy)(const void *))
{
    self->TreeConfig->KeyDestroy = KeyDestroy;
}
static void SetValueDestroy(AB_Tree_Map *self, void(*ValueDestroy)(const void *))
{
    self->TreeConfig->ValueDestroy = ValueDestroy;
}
static void GetFoundValue(AB_Tree_Map *self, void *ValueContainer)
{
    memcpy(ValueContainer, self->TreeConfig->privae_value, self->TreeConfig->value_size);
}
