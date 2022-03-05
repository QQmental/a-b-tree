#ifndef AB_NODE_ACC_MOD_H
#define AB_NODE_ACC_MOD_H
#include"AB_tree_algo.h"
#include<string.h>
struct node_arg
{
    AB_node *node;
    int ith_element;
};


typedef struct node_arg node_arg;

static inline void *Ptr_ith_Key(AB_Tree *self, AB_node *node, int ith);
static inline void *Ptr_ith_Child(AB_Tree *self, AB_node *node, int ith);
static inline node_arg Setnode_arg(AB_node *node, int ith_element);

void AB_node_WriteKey(AB_Tree *self, AB_node *node, const void *key, int ith);
void AB_node_WriteChild(AB_Tree *self, AB_node *node, const void *child, int ith);
void AB_node_MoveKeys(AB_Tree *self, node_arg dst_node, node_arg src_node, int moved_elem_cnt);
void AB_node_MoveChildren(AB_Tree *self, node_arg dst_node, node_arg src_node, int moved_elem_cnt);

void FreeAB_node(AB_Tree *self, AB_node *node);
AB_node *RequestAB_node(AB_Tree *self, int IsBottom);
AB_node *AccessAB_node_child(AB_Tree *self, AB_node *node, int ith_child);


static inline node_arg Setnode_arg(AB_node *node, int ith_element)
{
    node_arg ret = {.node = node, .ith_element = ith_element};
    return ret;
}








static inline void* Ptr_ith_Key(AB_Tree *self, AB_node *node, int ith)
{
    return (void*) ((char*)node + sizeof(AB_node) + self->key_size * ith);
}

static inline void *Ptr_ith_Child(AB_Tree *self, AB_node *node, int ith)
{
    size_t value_offset = sizeof(AB_node)+ (self->b-1) * self->key_size ;
    if (node->IsBottom == 0)
        return (void*) ((char*)node + value_offset + sizeof(AB_node*)* ith);
    else
        return (void*) ((char*)node + value_offset + self->value_size * ith);
}
#endif