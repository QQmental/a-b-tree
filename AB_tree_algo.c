#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "AB_tree_algo.h"
#include "algo_helper.h"
//#define BIN_SEARCH
//#define SHOWDIR


/*
typedef struct AB_node AB_node ;
typedef struct KV_pair KV_pair;

static inline void ShifChildrenRight(AB_Tree *self, AB_node *node, int pos, size_t child_size);
static inline void ShifKeysRight(AB_Tree *self, AB_node *node, int pos);

static inline void ShiftKeysLeft(AB_Tree *self, AB_node *node, int pos);
static inline void ShiftChildrenLeft(AB_Tree*, AB_node*, int, size_t);

static inline void *Ptr_ith_Key(AB_Tree *self, AB_node *node, int ith);
static inline void *Ptr_ith_Child(AB_Tree *self, AB_node *node, int ith);

static inline int IsFullNode(AB_Tree *self, AB_node *node);
static inline int GetKeyRank(AB_Tree *self, AB_node *node, void *key);

static AB_node *SplitNode(AB_Tree *self, AB_node *cur, KV_pair kv, int new_key_pos);
static AB_node *AddData(AB_Tree *self, AB_node *cur, KV_pair kv, int new_key_pos);
*/
static void ReplacedWithSuccesor(AB_Tree *self, AB_node *node_dst, int KeyRank_dst);


void AB_tree_Insert_algo(AB_Tree *self, KV_pair kv);
int AB_tree_Search_algo(AB_Tree *self, const void *key);
int AB_tree_Delete_algo(AB_Tree *self, const void *key);

static int
try_borrow_right(
        AB_Tree *self,
        AB_node *node,
        AB_node *parent,
        int branch_parent);


static int
try_borrow_left(
        AB_Tree *self,
        AB_node *node,
        AB_node *parent,
        int branch_parent);


static int try_merge_left_to_node(
    AB_Tree *self,
    AB_node *node,
    AB_node *parent,
    int branch_parent);

static int try_merge_right_to_node(
    AB_Tree *self,
    AB_node *node,
    AB_node *parent,
    int branch_parent);

int AB_tree_Search_algo(AB_Tree *self, const void *key)
{
    self->KeyIsFound = 0;
    self->SearchPathLength = 0;
    //self->privae_value = NULL;
    self->private_value.node = NULL;
    AB_node *node = self->root;
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
        }
        self->SearchBranch[self->SearchPathLength] = rank;
        self->SearchPath[self->SearchPathLength] = node;
        self->SearchPathLength++;
        if (node->IsBottom)
            break;

        node = AccessAB_node_child(self, node, rank);
    }

    if (self->KeyIsFound)
    {
        //self->privae_value
        //= Ptr_ith_Child(self, node, self->SearchBranch[self->SearchPathLength-1]);

        self->private_value = Setnode_arg(node, self->SearchBranch[self->SearchPathLength-1]);


    }
    return lv_node_key;
}


void AB_tree_Insert_algo(AB_Tree *self, KV_pair kv)
{
    //self->privae_value = NULL;
    //self->privae_value = NULL;
    self->private_value.node = NULL;
    AB_tree_Search_algo(self, kv.key);

    int level = self->SearchPathLength-1;

    AB_node *cur = self->SearchPath[level], *new_node;

    if (self->KeyIsFound)
    {
        //self->ValueDestroy(self->privae_value);
        AB_child_dtor(self, self->private_value.node, self->private_value.ith_element);
        //memcpy(self->privae_value, kv.value, self->value_size);
        AB_node_WriteChild(self, self->private_value.node, kv.value, self->private_value.ith_element);

        return ;
    }

    AB_node *chd;

    new_node = AddKeyValue(self, cur, kv, self->SearchBranch[level]);

    level -= 1;

    while(level != -1 && new_node)
    {
        AB_node_WriteChild(self, self->SearchPath[level], (void*)&new_node, self->SearchBranch[level]);

        new_node = AddLink(self, Setnode_arg(self->SearchPath[level], self->SearchBranch[level]), Setnode_arg(self->SearchPath[level+1], self->SearchPath[level+1]->KeyCount - 1));

        self->SearchPath[level+1]->KeyCount -= 1;

        level -= 1;
    }

    if (level == -1 && new_node)// the root was full and it splits
    {
        AB_node *tmp_root = self->root;

        self->root = RequestAB_node(self, 0);

        self->root->KeyCount = 1;

        AB_node_MoveKeys(self, Setnode_arg(self->root, 0), Setnode_arg(tmp_root, tmp_root->KeyCount-1), 1);

        AB_node_WriteChild(self, self->root, (void*)&tmp_root, 0);

        AB_node_WriteChild(self, self->root, (void*)&new_node, 1);

        tmp_root->KeyCount -= 1;
    }
}
int AB_tree_Delete_algo(AB_Tree *self, const void *key)
{
    AB_node *node_key;
    int level, rank_key;
    int depth_key = AB_tree_Search_algo(self, key);

    if (depth_key == -1) 
        return 0;
    
    level = self->SearchPathLength-1;
    rank_key = self->SearchBranch[depth_key];
    node_key = self->SearchPath[depth_key];

    if (node_key->IsBottom)
    {
        AB_key_dtor(self, node_key, rank_key);
        AB_child_dtor(self, node_key, rank_key);
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
        FreeAB_node(self, self->root);
        self->root = self->SearchPath[1];
    }
    return 1;


}

static void ReplacedWithSuccesor(AB_Tree *self, AB_node *node_dst, int KeyRank_dst)
{
    AB_node *succ = self->SearchPath[self->SearchPathLength-1];

    AB_key_dtor(self, node_dst, KeyRank_dst);

    //destroy value corresponding to the given key
    AB_child_dtor(self, succ, succ->KeyCount);

    AB_node_MoveKeys(self, Setnode_arg(node_dst, KeyRank_dst), Setnode_arg(succ, succ->KeyCount-1), 1);

    succ->KeyCount -= 1;

}



static int
try_borrow_right(
    AB_Tree *self,
    AB_node *node,
    AB_node *parent,
    int branch_parent)
{
    AB_node *sib_node = NULL;

    //no right sibling
    if (branch_parent == parent->KeyCount)
        return 0;

    sib_node = AccessAB_node_child(self, parent, branch_parent+1);

    if (sib_node->KeyCount == self->a-1)
        return 0;

    AB_node_MoveKeys(self, Setnode_arg(node, node->KeyCount), Setnode_arg(parent, branch_parent), 1);

    node->KeyCount += 1;

    AB_node_MoveChildren(self, Setnode_arg(node, node->KeyCount), Setnode_arg(sib_node, 0), 1);

    AB_node_MoveKeys(self, Setnode_arg(parent, branch_parent), Setnode_arg(sib_node, 0), 1);

    ShiftKeysLeft(self, sib_node, 0);
    
    ShiftChildrenLeft(self, sib_node, 0);
    
    sib_node->KeyCount -= 1;

    return 1;
}

static int
try_borrow_left(
    AB_Tree *self,
    AB_node *node,
    AB_node *parent,
    int branch_parent)
{
    AB_node *sib_node = NULL;

    //no left sibling
    if (branch_parent == 0)
        return 0;

    sib_node = AccessAB_node_child(self, parent, branch_parent-1);

    if (sib_node->KeyCount == self->a-1)
        return 0;

    ShifKeysRight(self, node, 0);
    
    ShifChildrenRight(self, node, 0);
 
    AB_node_MoveKeys(self, Setnode_arg(node, 0), Setnode_arg(parent, branch_parent-1), 1);

   
    AB_node_MoveChildren(self, Setnode_arg(node, 0), Setnode_arg(sib_node, sib_node->KeyCount), 1);
    
    node->KeyCount += 1;

    AB_node_MoveKeys(self, Setnode_arg(parent, branch_parent-1), Setnode_arg(sib_node, sib_node->KeyCount-1), 1);

    sib_node->KeyCount -= 1;

    return 1;
}



static int try_merge_right_to_node(
    AB_Tree *self,
    AB_node *node,
    AB_node *parent,
    int branch_parent)
{

    AB_node *sib_node = NULL;

    //no right sibling
    if (branch_parent == parent->KeyCount)
        return 0;
 
    sib_node = AccessAB_node_child(self, parent, branch_parent+1);

    AB_node_MoveKeys(self, Setnode_arg(node, node->KeyCount), Setnode_arg(parent, branch_parent), 1);

    AB_node_MoveKeys(self, Setnode_arg(node, node->KeyCount+1), Setnode_arg(sib_node, 0), sib_node->KeyCount);

    AB_node_MoveChildren(self, 
                        Setnode_arg(node, node->KeyCount+1),
                        Setnode_arg(sib_node, 0), 
                        sib_node->KeyCount+1);


    node->KeyCount += (sib_node->KeyCount+1);

    FreeAB_node(self, sib_node);

    ShiftKeysLeft(self, parent, branch_parent);
    
    ShiftChildrenLeft(self, parent, branch_parent+1);
    
    parent->KeyCount -= 1;

    
    return 1;
}








static int try_merge_left_to_node(
    AB_Tree *self,
    AB_node *node,
    AB_node *parent,
    int branch_parent)
{

    AB_node *sib_node = NULL;

    //no left sibling
    if (branch_parent == 0)
        return 0;

    sib_node = AccessAB_node_child(self, parent, branch_parent-1);


    AB_node_MoveKeys(self, Setnode_arg(node, sib_node->KeyCount+1), Setnode_arg(node, 0), node->KeyCount);

    AB_node_MoveChildren(self, 
                        Setnode_arg(node, sib_node->KeyCount+1), 
                        Setnode_arg(node, 0), 
                        node->KeyCount+1);

    AB_node_MoveKeys(self, Setnode_arg(node, sib_node->KeyCount), Setnode_arg(parent, branch_parent-1), 1);

    AB_node_MoveKeys(self, Setnode_arg(node, 0), Setnode_arg(sib_node, 0), sib_node->KeyCount);

    AB_node_MoveChildren(self, 
                        Setnode_arg(node, 0), 
                        Setnode_arg(sib_node, 0), 
                        sib_node->KeyCount+1);

    node->KeyCount += (sib_node->KeyCount+1);

    FreeAB_node(self, sib_node);

    ShiftKeysLeft(self, parent, branch_parent-1);
    
    ShiftChildrenLeft(self, parent, branch_parent-1);
    
    parent->KeyCount -= 1;
    
    return 1 ;
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
            #ifdef SHOWDIR
                printf("back\n");
            #endif
            
            self->SearchBranch[level] = 0;
            level--;
            continue;
        }
        
        if (self->SearchBranch[level] > 0)
            printf("%d\n",*(int*)Ptr_ith_Key(self, self->SearchPath[level], self->SearchBranch[level]-1));

        #ifdef SHOWDIR
           printf("down\n");
        #endif
        self->SearchPath[level+1] = AccessAB_node_child(self, self->SearchPath[level], self->SearchBranch[level]);
        //= *(AB_node**)Ptr_ith_Child(self, self->SearchPath[level], self->SearchBranch[level]);


        self->SearchBranch[level]++;
        level++;
    }
}
