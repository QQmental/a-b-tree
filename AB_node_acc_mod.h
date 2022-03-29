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

void *Ptr_ith_Key(AB_Tree *self, AB_node *node, int ith);
void *Ptr_ith_Child(AB_Tree *self, AB_node *node, int ith);
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









#endif