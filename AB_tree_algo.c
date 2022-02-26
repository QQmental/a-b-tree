#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "AB_tree_algo.h"
#include "algo_helper.h"
//#define BIN_SEARCH

/*
typedef struct AB_internel_node AB_internel_node ;
typedef struct KV_pair KV_pair;

static inline void ShifChildrenRight(AB_Tree *self, AB_internel_node *node, int pos, size_t child_size);
static inline void ShifKeysRight(AB_Tree *self, AB_internel_node *node, int pos);

static inline void ShiftKeysLeft(AB_Tree *self, AB_internel_node *node, int pos);
static inline void ShiftChildrenLeft(AB_Tree*, AB_internel_node*, int, size_t);

static inline void *Ptr_ith_Key(AB_Tree *self, AB_internel_node *node, int ith);
static inline void *Ptr_ith_Child(AB_Tree *self, AB_internel_node *node, int ith);

static inline int IsFullNode(AB_Tree *self, AB_internel_node *node);
static inline int GetKeyRank(AB_Tree *self, AB_internel_node *node, void *key);

static AB_internel_node *SplitNode(AB_Tree *self, AB_internel_node *cur, KV_pair kv, int new_key_pos);
static AB_internel_node *AddData(AB_Tree *self, AB_internel_node *cur, KV_pair kv, int new_key_pos);
*/



void AB_tree_Insert_algo(AB_Tree *self, KV_pair kv);
int AB_tree_Search_algo(AB_Tree *self, const void *key);
int AB_tree_Delete_algo(AB_Tree *self, const void *key);



int AB_tree_Search_algo(AB_Tree *self, const void *key)
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

        AB_node_WriteChild(self, cur, (void*)&new_node, self->SearchBranch[level]);
    }

    if (level == -1 && new_node)// the root was full and it splits
    {
        AB_internel_node *tmp_root = self->root;
        self->root = MakeNode(self, sizeof(AB_internel_node*), 0);
        self->root->KeyCount = 1;

        AB_node_WriteKey(self, self->root, Ptr_ith_Key(self, tmp_root, tmp_root->KeyCount-1), 0);

        AB_node_WriteChild(self, self->root, (void*)&tmp_root, 0);

        AB_node_WriteChild(self, self->root, (void*)&new_node, 1);

        tmp_root->KeyCount -= 1;
    }
}
int AB_tree_Delete_algo(AB_Tree *self, const void *key)
{
    AB_internel_node *node_key;
    int level, rank_key;
    int depth_key = AB_tree_Search_algo(self, key);

    if (depth_key == -1) 
        return 0;
    
    level = self->SearchPathLength-1;
    rank_key = self->SearchBranch[depth_key];
    node_key = self->SearchPath[depth_key];

    if (node_key->IsBottom)
    {
        self->KeyDestroy(Ptr_ith_Key(self, node_key, rank_key));
        self->ValueDestroy(Ptr_ith_Child(self, node_key, rank_key));
        ShiftKeysLeft(self, node_key, rank_key);
        ShiftChildrenLeft(self, node_key, rank_key);
        node_key->KeyCount -= 1;
    }
    else
    {
        ReplacedWithSuccesor(self, node_key, rank_key);
    }
    
    if (self->SearchPathLength == 1)
        return 1;

    while(level && self->SearchPath[level]->KeyCount < self->a-1)
    {
        int try_borrow = try_borrow_left(self, self->SearchPath[level], self->SearchPath[level-1], self->SearchBranch[level-1]);
        if (try_borrow)
            return 1;

        try_borrow = try_borrow_right(self, self->SearchPath[level], self->SearchPath[level-1], self->SearchBranch[level-1]);
        if (try_borrow)
            return 1;

        int try_merge = try_merge_left_to_node(self, self->SearchPath[level], self->SearchPath[level-1], self->SearchBranch[level-1]);
        if (try_merge == 0)
            try_merge = try_merge_right_to_node(self, self->SearchPath[level], self->SearchPath[level-1], self->SearchBranch[level-1]);

        level -= 1;
    }
    if (level == 0 && self->root->KeyCount == 0)
    {
        free(self->root);
        self->root = self->SearchPath[1];
    }
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
            //printf("back\n");
            self->SearchBranch[level] = 0;
            level--;
            continue;
        }
        
        if (self->SearchBranch[level] > 0)
            printf("%d\n",*(int*)Ptr_ith_Key(self, self->SearchPath[level], self->SearchBranch[level]-1));
        //printf("down\n");
        self->SearchPath[level+1]
        = *(AB_internel_node**)Ptr_ith_Child(self, self->SearchPath[level], self->SearchBranch[level]);

        self->SearchBranch[level]++;
        level++;
    }
}
