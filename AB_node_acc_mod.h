#ifndef AB_NODE_ACC_MOD_H
#define AB_NODE_ACC_MOD_H
#include"AB_tree_algo.h"
#include<string.h>
static inline void *Ptr_ith_Key(AB_Tree *self, AB_internel_node *node, int ith);
static inline void *Ptr_ith_Child(AB_Tree *self, AB_internel_node *node, int ith);


void AB_node_WriteKey(AB_Tree *self, AB_internel_node *node, const void *key, int ith);
void AB_node_WriteChild(AB_Tree *self, AB_internel_node *node, const void *child, int ith);
void AB_node_MoveKeys(AB_Tree *self, AB_internel_node *dst_node, AB_internel_node *src_node, int dst_ith_key, int src_ith_key, int moved_elem_cnt);
void AB_node_MoveChildren(AB_Tree *self, AB_internel_node *dst_node, AB_internel_node *src_node, int dst_ith_child, int src_ith_child, int moved_elem_cnt);













static inline void* Ptr_ith_Key(AB_Tree *self, AB_internel_node *node, int ith)
{
    return (void*) ((char*)node + sizeof(AB_internel_node) + self->key_size * ith);
}

static inline void *Ptr_ith_Child(AB_Tree *self, AB_internel_node *node, int ith)
{
    size_t value_offset = sizeof(AB_internel_node)+ (self->b-1) * self->key_size ;
    if (node->IsBottom == 0)
        return (void*) ((char*)node + value_offset + sizeof(AB_internel_node*)* ith);
    else
        return (void*) ((char*)node + value_offset + self->value_size * ith);
}
#endif