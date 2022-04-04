#ifndef ALGO_HELPER_H_INCLUDED
#define ALGO_HELPER_H_INCLUDED
#include"AB_tree_algo.h"
#include"AB_node_acc_mod.h"
#include"AB_KV_util.h"
#include"AB_node.h"
#include<string.h>

typedef struct AB_node AB_node ;
typedef struct KV_pair KV_pair;

static inline void ShifChildrenRight(AB_Tree *self, AB_node *node, int pos);
static inline void ShifKeysRight(AB_Tree *self, AB_node *node, int pos);

static inline void ShiftKeysLeft(AB_Tree *self, AB_node *node, int dst);
static inline void ShiftChildrenLeft(AB_Tree *self, AB_node *node, int dst);

static inline int IsFullNode(AB_Tree *self, AB_node *node);
static inline int GetKeyRank(AB_Tree *self, AB_node *node, const void *key);

static AB_node *SplitNode(AB_Tree *self, AB_node *cur, int new_key_pos);
static AB_node *AddKeyValue(AB_Tree *self, AB_node *cur, KV_pair kv, int new_key_pos);
static AB_node *AddLink(AB_Tree *self, node_arg dst_node, node_arg src_node);

static inline int GetKeyRank(AB_Tree *self, AB_node *node, const void *key)
{
    #ifndef BIN_SEARCH
    int i = 0 ;
    for(; i < node->KeyCount ; i++)
    {
        //int cmp = self->KeyComp(key, Ptr_ith_Key(self, node, i));
        int cmp = AB_CompareKey(self, key, node, i);
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

static inline int IsFullNode(AB_Tree *self, AB_node *node)
{
    
    
    
    
    
    
    
    
    
    
    return node->KeyCount == self->b- 1;
}





static inline void ShifKeysRight(AB_Tree *self, AB_node *node, int pos)
{
    AB_node_MoveKeys(self, Setnode_arg(node, pos+1), Setnode_arg(node, pos), node->KeyCount-pos);

}

static inline void ShifChildrenRight(AB_Tree *self, AB_node *node, int pos)
{
    AB_node_MoveChildren(self, Setnode_arg(node, pos+1), Setnode_arg(node, pos), node->KeyCount-pos+1);
}


static inline void ShiftKeysLeft(AB_Tree *self, AB_node *node, int dst)
{
    AB_node_MoveKeys(self, Setnode_arg(node, dst), Setnode_arg(node, dst+1), node->KeyCount-dst-1);    
}

static inline void ShiftChildrenLeft(AB_Tree *self, AB_node *node, int dst)
{
    AB_node_MoveChildren(self, Setnode_arg(node, dst), Setnode_arg(node, dst+1), node->KeyCount-dst);     
}

static AB_node *SplitNode(AB_Tree *self, AB_node *cur, int new_key_pos)
{
    AB_node *new_node = RequestAB_node(self, cur->IsBottom);

    cur->KeyCount = (self->b)/2;
    new_node->KeyCount = self->b - cur->KeyCount;

    if (new_key_pos < (self->b)/2) //new key will reside in old node
    {
        AB_node_MoveKeys(self, Setnode_arg(new_node, 0), Setnode_arg(cur, cur->KeyCount-1), new_node->KeyCount);

        AB_node_MoveChildren(self, 
                            Setnode_arg(new_node, 0), 
                            Setnode_arg(cur, cur->KeyCount-1), 
                            new_node->KeyCount + 1);

        ShifKeysRight(self, cur, new_key_pos);

        ShifChildrenRight(self, cur, new_key_pos);
    }
    else //new key will reside in new node
    {
        AB_node_MoveKeys(self, Setnode_arg(new_node, 0), Setnode_arg(cur, cur->KeyCount), new_key_pos - cur->KeyCount);

        AB_node_MoveKeys(self, 
                        Setnode_arg(new_node, new_key_pos - cur->KeyCount + 1), 
                        Setnode_arg(cur, new_key_pos), 
                        cur->KeyCount + new_node->KeyCount - new_key_pos - 1);

        AB_node_MoveChildren(self, 
                            Setnode_arg(new_node, 0), 
                            Setnode_arg(cur, cur->KeyCount), 
                            new_key_pos - cur->KeyCount);

        AB_node_MoveChildren(self, 
                            Setnode_arg(new_node, new_key_pos - cur->KeyCount + 1), 
                            Setnode_arg(cur, new_key_pos), 
                            cur->KeyCount + new_node->KeyCount - new_key_pos);
    
    }

    return new_node ;
}

static AB_node *AddKeyValue(AB_Tree *self, AB_node *cur, KV_pair kv, int new_key_pos)
{
    if (IsFullNode(self, cur))
    {
        AB_node *new_node = SplitNode(self, cur, new_key_pos);

        int pos = new_key_pos ;
        
        AB_node *targ_node = cur;

        if (new_key_pos >= (self->b)/2)
        {
            pos = new_key_pos-cur->KeyCount;

            targ_node = new_node;
        }

        AB_node_WriteKey(self, targ_node, kv.key, pos);

        AB_node_WriteChild(self, targ_node, kv.value, pos);

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

static AB_node *AddLink(AB_Tree *self, node_arg dst_node, node_arg src_node)
{

    if (IsFullNode(self, dst_node.node))
    {
        AB_node *new_node = SplitNode(self, dst_node.node, dst_node.ith_element);

        int pos = dst_node.ith_element ;
        
        AB_node *targ_node = dst_node.node;

        if (dst_node.ith_element >= (self->b)/2)
        {
            pos = dst_node.ith_element - dst_node.node->KeyCount;

            targ_node = new_node;
        }

        AB_node_MoveKeys(self, Setnode_arg(targ_node, pos), src_node, 1);

        AB_node_WriteChild(self, targ_node, (void*)&src_node.node, pos);

        return new_node;
    }
    else
    {
        ShifKeysRight(self, dst_node.node, dst_node.ith_element);

        ShifChildrenRight(self, dst_node.node, dst_node.ith_element);

        AB_node_MoveKeys(self, dst_node, src_node, 1);

        AB_node_WriteChild(self, dst_node.node, (void*)&src_node, dst_node.ith_element);
          
        dst_node.node->KeyCount += 1;
        return NULL;
    }
}



#endif // ALGO_HELPER_H_INCLUDED
