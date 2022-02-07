#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "AB_tree_algo.h"
#include "algo_helper.h"
//#define BIN_SEARCH

/*
typedef struct AB_internel_node AB_internel_node ;
typedef struct KV_pair KV_pair;

static inline void ShifChildrenBackward(AB_Tree *self, AB_internel_node *node, int pos, size_t child_size);
static inline void ShifKeysBackward(AB_Tree *self, AB_internel_node *node, int pos);

static inline void ShiftKeysForward(AB_Tree *self, AB_internel_node *node, int pos);
static inline void ShiftChildrenForward(AB_Tree*, AB_internel_node*, int, size_t);

static inline void *Ptr_ith_Key(AB_Tree *self, AB_internel_node *node, int ith);
static inline void *Ptr_ith_Child(AB_Tree *self, AB_internel_node *node, int ith);

static inline int IsFullNode(AB_Tree *self, AB_internel_node *node);
static inline int GetKeyRank(AB_Tree *self, AB_internel_node *node, void *key);

static AB_internel_node *SplitNode(AB_Tree *self, AB_internel_node *cur, KV_pair kv, int new_key_pos);
static AB_internel_node *AddData(AB_Tree *self, AB_internel_node *cur, KV_pair kv, int new_key_pos);
*/
void AB_tree_Insert_algo(AB_Tree *self, KV_pair kv);
int AB_tree_Search_algo(AB_Tree *self, void *key);
void AB_tree_Delete_algo(AB_Tree *self, void *key);

static int
try_borrow_higher(
        AB_Tree *self,
        AB_internel_node *node,
        AB_internel_node *parent,
        int branch_parent);


static int
try_borrow_lower(
        AB_Tree *self,
        AB_internel_node *node,
        AB_internel_node *parent,
        int branch_parent);


int AB_tree_Search_algo(AB_Tree *self, void *key)
{
    self->KeyIsFound = 0;
    self->SearchPathLength = 0;
    self->privae_key = self->privae_value = NULL;
    AB_internel_node *node = self->root;
    int lv_node_key = -1, rank = 0;

    while(1)
    {
        if (self->KeyIsFound)
            rank = node->KeyCount;
        else
            rank = GetKeyRank(self, node, key);

        if (self->KeyIsFound && lv_node_key == -1)
        {
            lv_node_key = self->SearchPathLength;
            self->privae_key = Ptr_ith_Key(self, node, rank);
        }

        self->SearchBranch[self->SearchPathLength] = rank;
        self->SearchPath[self->SearchPathLength] = node;
        self->SearchPathLength++;
        if (node->IsBottom)
            break;

        node = *(AB_internel_node**)Ptr_ith_Child(self, node, rank);
    }

    if (self->KeyIsFound)
    {
        self->privae_value
        = Ptr_ith_Child(self, node, self->SearchBranch[self->SearchPathLength-1]);
    }

    return lv_node_key;
}


void AB_tree_Insert_algo(AB_Tree *self, KV_pair kv)
{
    self->privae_value = NULL;
    AB_tree_Search_algo(self, kv.key);

    int level = self->SearchPathLength-1;

    AB_internel_node *cur = self->SearchPath[level], *new_node;

    if (self->KeyIsFound)
    {
        self->ValueDestroy(self->privae_value);
        memcpy(self->privae_value, kv.value, self->value_size);
        return ;
    }

    AB_internel_node *chd;
    while(1)
    {
        new_node = AddData(self, cur, kv, self->SearchBranch[level]);
        level -= 1;

        if (new_node == NULL || level == -1)
            break;
        kv.key = Ptr_ith_Key(self, self->SearchPath[level+1], self->SearchPath[level+1]->KeyCount-1);
        self->SearchPath[level+1]->KeyCount -= 1;
        chd = cur;
        kv.value = (void*)&chd;

        cur = self->SearchPath[level];

        memcpy(Ptr_ith_Child(self, cur, self->SearchBranch[level]),(void*)&new_node, sizeof(AB_internel_node*));
    }

    if (level == -1 && new_node)// the root was full and it splits
    {
        AB_internel_node *tmp_root = self->root;
        self->root = MakeNode(self, sizeof(AB_internel_node*), 0);
        self->root->KeyCount = 1;
        memcpy(Ptr_ith_Key(self, self->root, 0),
               Ptr_ith_Key(self, tmp_root, tmp_root->KeyCount-1),
               self->key_size);

        memcpy(Ptr_ith_Child(self, self->root, 0),
               (void*)&tmp_root,
               sizeof(AB_internel_node*));

        memcpy(Ptr_ith_Child(self, self->root, 1),
               (void*)&new_node,
               sizeof(AB_internel_node*));
        tmp_root->KeyCount -= 1;
    }
}
void AB_tree_Delete_algo(AB_Tree *self, void *key)
{

    AB_internel_node *node_key;
    int level, rank_key;
    int depth_key = AB_tree_Search_algo(self, key);
    if (depth_key == -1)
        return ;

    level = self->SearchPathLength-1;
    rank_key = self->SearchBranch[depth_key];
    node_key = self->SearchPath[depth_key];

    if (node_key->IsBottom)
    {
        self->KeyDestroy(Ptr_ith_Key(self, node_key, rank_key));
        self->ValueDestroy(Ptr_ith_Child(self, node_key, rank_key));
        ShiftKeysForward(self, node_key, rank_key);
        ShiftChildrenForward(self, node_key, rank_key, self->value_size);
    }
    else
    {
        AB_internel_node *last_node = self->SearchPath[self->SearchPathLength-1];

        self->KeyDestroy(Ptr_ith_Key(self, node_key, rank_key));

        //destroy value corresponding to the given key
        self->ValueDestroy(Ptr_ith_Child(self, last_node, last_node->KeyCount));

        memcpy(Ptr_ith_Key(self, node_key, rank_key),
               Ptr_ith_Key(self, last_node, last_node->KeyCount-1),
               self->key_size);
        last_node->KeyCount-=1;
    }
}

static int
try_borrow_higher(
    AB_Tree *self,
    AB_internel_node *node,
    AB_internel_node *parent,
    int branch_parent)
{
    AB_internel_node *sib_node = NULL;
    if (branch_parent == parent->KeyCount)
        return 0;
    sib_node = *(AB_internel_node**)Ptr_ith_Child(self, parent, branch_parent+1);
    if (sib_node->KeyCount < self->a-1)
        return 0;

    size_t child_size = sizeof(AB_internel_node*);
    if (node->IsBottom)
        child_size = self->value_size;

    memcpy(Ptr_ith_Key(self, node, node->KeyCount),
           Ptr_ith_Key(self, parent, branch_parent),
           self->key_size);

    node->KeyCount += 1;

    memcpy(Ptr_ith_Child(self, node, node->KeyCount),
           Ptr_ith_Child(self, sib_node, 0),
           child_size);

    memcpy(Ptr_ith_Key(self, parent, branch_parent),
           Ptr_ith_Key(self, sib_node, 0),
           self->key_size);

    ShiftKeysForward(self, sib_node, 0);
    ShiftChildrenForward(self, sib_node, 0, child_size);
    sib_node->KeyCount -= 1;

    return 1;
}

static int
try_borrow_lower(
    AB_Tree *self,
    AB_internel_node *node,
    AB_internel_node *parent,
    int branch_parent)
{
    AB_internel_node *sib_node = NULL;
    if (branch_parent == 0)
        return 0;
    sib_node = *(AB_internel_node**)Ptr_ith_Child(self, parent, branch_parent-1);
    if (sib_node->KeyCount < self->a-1)
        return 0;

    size_t child_size = sizeof(AB_internel_node*);
    if (node->IsBottom)
        child_size = self->value_size;





    return 1;
}













void tra(AB_Tree *self)
{
    self->SearchPathLength = 1 ;
    self->SearchPath[0] = self->root;
    int i = 0, level = 0;
    for(i = 0 ; i < 32 ; i++)
        self->SearchBranch[i] = 0;
    while(level >= 0)
    {
        if (self->SearchPath[level]->IsBottom || self->SearchBranch[level] > self->SearchPath[level]->KeyCount)
        {
            if (self->SearchPath[level]->IsBottom)
            {
                for(i = 0 ; i < self->SearchPath[level]->KeyCount ; i++)
                    printf("%d\n",*(int*)Ptr_ith_Key(self, self->SearchPath[level], i));
            }
            self->SearchBranch[level] = 0;
            level--;
            continue;
        }

        if (self->SearchBranch[level] > 0)
            printf("%d\n",*(int*)Ptr_ith_Key(self, self->SearchPath[level], self->SearchBranch[level]-1));

        self->SearchPath[level+1]
        = *(AB_internel_node**)Ptr_ith_Child(self, self->SearchPath[level], self->SearchBranch[level]);

        self->SearchBranch[level]++;
        level++;
    }
}
