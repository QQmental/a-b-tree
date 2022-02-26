#ifndef ALGO_HELPER_H_INCLUDED
#define ALGO_HELPER_H_INCLUDED
#include"AB_tree_algo.h"
#include"AB_node_acc_mod.h"
#include<string.h>

typedef struct AB_internel_node AB_internel_node ;
typedef struct KV_pair KV_pair;

static inline void ShifChildrenRight(AB_Tree *self, AB_internel_node *node, int pos);
static inline void ShifKeysRight(AB_Tree *self, AB_internel_node *node, int pos);

static inline void ShiftKeysLeft(AB_Tree *self, AB_internel_node *node, int dst);
static inline void ShiftChildrenLeft(AB_Tree *self, AB_internel_node *node, int dst);

static inline int IsFullNode(AB_Tree *self, AB_internel_node *node);
static inline int GetKeyRank(AB_Tree *self, AB_internel_node *node, void *key);

static AB_internel_node *SplitNode(AB_Tree *self, AB_internel_node *cur, KV_pair kv, int new_key_pos);
static AB_internel_node *AddData(AB_Tree *self, AB_internel_node *cur, KV_pair kv, int new_key_pos);
static void ReplacedWithSuccesor(AB_Tree *self, AB_internel_node *node_dst, int KeyRank_dst);

static int
try_borrow_right(
        AB_Tree *self,
        AB_internel_node *node,
        AB_internel_node *parent,
        int branch_parent);

static int
try_borrow_left(
        AB_Tree *self,
        AB_internel_node *node,
        AB_internel_node *parent,
        int branch_parent);

static int try_merge_left_to_node(
    AB_Tree *self,
    AB_internel_node *node,
    AB_internel_node *parent,
    int branch_parent);

static int try_merge_right_to_node(
    AB_Tree *self,
    AB_internel_node *node,
    AB_internel_node *parent,
    int branch_parent);

static inline int GetKeyRank(AB_Tree *self, AB_internel_node *node, void *key)
{
    #ifndef BIN_SEARCH
    int i = 0 ;
    for(; i < node->KeyCount ; i++)
    {
        int cmp = self->KeyComp(key, Ptr_ith_Key(self, node, i));
        if (cmp == -1)
            break;

        if (cmp == 0)
        {
            self->KeyIsFound = 1;
            break;
        }
    }
    return i ;
    #else
    int ret = node->KeyCount, l = 0, r = node->KeyCount -1;
    while(l <= r)
    {
        int cmp = self->KeyComp(key, Ptr_ith_Key(self, node, (l+r)>>1));
        if (cmp == 0)
        {
            self->KeyIsFound = 1;
            ret = (l+r)>>1;
            break;
        }
        else if (cmp == 1)
        {
            l = ((l+r)>>1)+1;
        }
        else
        {
            ret = (l+r)>>1;
            r = ((l+r)>>1)-1;
        }
    }

    return ret;
    #endif

}

static inline int IsFullNode(AB_Tree *self, AB_internel_node *node)
{
    return node->KeyCount == self->b-1;
}





static inline void ShifKeysRight(AB_Tree *self, AB_internel_node *node, int pos)
{
    AB_node_MoveKeys(self, node, node, pos+1, pos, (node->KeyCount-pos));
}

static inline void ShifChildrenRight(AB_Tree *self, AB_internel_node *node, int pos)
{
    AB_node_MoveChildren(self, node, node, pos+1, pos, (node->KeyCount-pos+1)) ;    
}


static inline void ShiftKeysLeft(AB_Tree *self, AB_internel_node *node, int dst)
{
    AB_node_MoveKeys(self, node, node, dst, dst+1, (node->KeyCount-dst-1)) ;      
}

static inline void ShiftChildrenLeft(AB_Tree *self, AB_internel_node *node, int dst)
{
    AB_node_MoveChildren(self, node, node, dst, dst+1, (node->KeyCount-dst));       
}

static AB_internel_node *SplitNode(AB_Tree *self, AB_internel_node *cur, KV_pair kv, int new_key_pos)
{
    size_t child_size = sizeof(AB_internel_node *);

    if (cur->IsBottom)
        child_size = self->value_size ;

    AB_internel_node *new_node = MakeNode(self, child_size, cur->IsBottom);
    cur->KeyCount = (self->b)/2;
    new_node->KeyCount = self->b - cur->KeyCount;

    if (new_key_pos < (self->b)/2) //new key will reside in old node
    {
        AB_node_MoveKeys(self, new_node, cur, 0, cur->KeyCount-1, new_node->KeyCount);

        AB_node_MoveChildren(self, new_node, cur, 0, cur->KeyCount-1, (new_node->KeyCount + 1));

        ShifKeysRight(self, cur, new_key_pos);

        ShifChildrenRight(self, cur, new_key_pos);

        AB_node_WriteKey(self, cur, kv.key, new_key_pos);

        AB_node_WriteChild(self, cur, kv.value, new_key_pos);


    }
    else //new key will reside in new node
    {

        AB_node_MoveKeys(self, new_node, cur, 0, cur->KeyCount, (new_key_pos - cur->KeyCount));

        AB_node_WriteKey(self, new_node, kv.key, new_key_pos-cur->KeyCount);

        AB_node_MoveKeys(self, new_node, cur, new_key_pos - cur->KeyCount + 1, new_key_pos, (cur->KeyCount + new_node->KeyCount - new_key_pos - 1));



        AB_node_MoveChildren(self, new_node, cur, 0, cur->KeyCount, (new_key_pos - cur->KeyCount));

        AB_node_WriteChild(self, new_node, kv.value, new_key_pos-cur->KeyCount);

        AB_node_MoveChildren(self, new_node, cur, new_key_pos - cur->KeyCount + 1, new_key_pos, (cur->KeyCount + new_node->KeyCount - new_key_pos));
    }

    return new_node ;
}

static AB_internel_node *AddData(AB_Tree *self, AB_internel_node *cur, KV_pair kv, int new_key_pos)
{
    size_t child_size = sizeof(AB_internel_node *);

    if (cur->IsBottom)
        child_size = self->value_size ;

    if (IsFullNode(self, cur))
    {

        AB_internel_node *new_node = SplitNode(self, cur, kv, new_key_pos);

        return new_node;

    }
    else
    {
        ShifKeysRight(self, cur, new_key_pos);

        ShifChildrenRight(self, cur, new_key_pos);

        AB_node_WriteKey(self, cur, kv.key, new_key_pos);

        AB_node_WriteChild(self, cur, kv.value, new_key_pos);
        
        
        cur->KeyCount += 1;
        return NULL;
    }
}

static void ReplacedWithSuccesor(AB_Tree *self, AB_internel_node *node_dst, int KeyRank_dst)
{
    AB_internel_node *succ = self->SearchPath[self->SearchPathLength-1];
    self->KeyDestroy(Ptr_ith_Key(self, node_dst, KeyRank_dst));
    //destroy value corresponding to the given key
    self->ValueDestroy(Ptr_ith_Child(self, succ, succ->KeyCount));
    AB_node_WriteKey(self, node_dst, Ptr_ith_Key(self, succ, succ->KeyCount-1), KeyRank_dst);
    succ->KeyCount -= 1;

}

static int
try_borrow_right(
    AB_Tree *self,
    AB_internel_node *node,
    AB_internel_node *parent,
    int branch_parent)
{
    AB_internel_node *sib_node = NULL;

    //no right sibling
    if (branch_parent == parent->KeyCount)
        return 0;
    sib_node = *(AB_internel_node**)Ptr_ith_Child(self, parent, branch_parent+1);
    if (sib_node->KeyCount == self->a-1)
        return 0;

    AB_node_WriteKey(self, node, Ptr_ith_Key(self, parent, branch_parent), node->KeyCount);

    node->KeyCount += 1;

    AB_node_WriteChild(self, node, Ptr_ith_Child(self, sib_node, 0), node->KeyCount);

    AB_node_WriteKey(self, parent, Ptr_ith_Key(self, sib_node, 0), branch_parent);

    ShiftKeysLeft(self, sib_node, 0);
    ShiftChildrenLeft(self, sib_node, 0);
    sib_node->KeyCount -= 1;

    return 1;
}

static int
try_borrow_left(
    AB_Tree *self,
    AB_internel_node *node,
    AB_internel_node *parent,
    int branch_parent)
{
    AB_internel_node *sib_node = NULL;

    //no left sibling
    if (branch_parent == 0)
        return 0;
    sib_node = *(AB_internel_node**)Ptr_ith_Child(self, parent, branch_parent-1);
    if (sib_node->KeyCount == self->a-1)
        return 0;

    ShifKeysRight(self, node, 0);
    ShifChildrenRight(self, node, 0);
    AB_node_WriteKey(self, node, Ptr_ith_Key(self, parent, branch_parent-1), 0);
    AB_node_WriteChild(self, node, Ptr_ith_Child(self, sib_node, sib_node->KeyCount), 0);
    node->KeyCount += 1;

    AB_node_WriteKey(self, parent, Ptr_ith_Key(self, sib_node, sib_node->KeyCount-1), branch_parent-1);
    sib_node->KeyCount -= 1;

    return 1;
}



static int try_merge_right_to_node(
    AB_Tree *self,
    AB_internel_node *node,
    AB_internel_node *parent,
    int branch_parent)
{

    AB_internel_node *sib_node = NULL;

    //no right sibling
    if (branch_parent == parent->KeyCount)
        return 0;
    sib_node = *(AB_internel_node**)Ptr_ith_Child(self, parent, branch_parent+1);
    
    AB_node_WriteKey(self, node, Ptr_ith_Key(self, parent, branch_parent), node->KeyCount);
    AB_node_MoveKeys(self, node, sib_node, node->KeyCount+1, 0, sib_node->KeyCount);
    AB_node_MoveChildren(self, node, sib_node, node->KeyCount+1, 0, sib_node->KeyCount+1);
    node->KeyCount += (sib_node->KeyCount+1);
    free(sib_node);

    ShiftKeysLeft(self, parent, branch_parent);
    ShiftChildrenLeft(self, parent, branch_parent+1);
    parent->KeyCount -= 1;

    
    return 1;
}








static int try_merge_left_to_node(
    AB_Tree *self,
    AB_internel_node *node,
    AB_internel_node *parent,
    int branch_parent)
{

    AB_internel_node *sib_node = NULL;

    //no left sibling
    if (branch_parent == 0)
        return 0;
    sib_node = *(AB_internel_node**)Ptr_ith_Child(self, parent, branch_parent-1);


    AB_node_MoveKeys(self, node, node, sib_node->KeyCount + 1, 0, node->KeyCount);
    AB_node_MoveChildren(self, node, node, sib_node->KeyCount+1, 0, node->KeyCount+1);

    AB_node_WriteKey(self, node, Ptr_ith_Key(self, parent, branch_parent-1), sib_node->KeyCount);

    AB_node_MoveKeys(self, node, sib_node, 0, 0, sib_node->KeyCount);
    AB_node_MoveChildren(self, node, sib_node, 0, 0, sib_node->KeyCount+1);

    node->KeyCount += (sib_node->KeyCount+1);

    free(sib_node);

    ShiftKeysLeft(self, parent, branch_parent-1);
    ShiftChildrenLeft(self, parent, branch_parent-1);
    parent->KeyCount -= 1;
    return 1 ;
}


#endif // ALGO_HELPER_H_INCLUDED
