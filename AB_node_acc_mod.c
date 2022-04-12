#include"AB_node_acc_mod.h"
#include"AB_node.h"
#include"stdio.h"

#define BIN_OPTION(bin_val, true_if_1, true_if_0)\
(true_if_1*bin_val | true_if_0*(bin_val^1))


static void AB_node_datacpy(AB_Tree *self, void *dst, void *src, size_t byte_cnt);
void AB_node_WriteKey(AB_Tree *self, AB_node *node, const void *key, int ith);
void AB_node_WriteChild(AB_Tree *self, AB_node *node, const void *child, int ith);
void AB_node_MoveChildren(AB_Tree *self, node_arg dst_node, node_arg src_node, int moved_elem_cnt);
void AB_node_MoveKeys(AB_Tree *self, node_arg dst_node, node_arg src_node, int moved_elem_cnt);

void FreeAB_node(AB_Tree *self, AB_node *node);
AB_node *RequestAB_node(AB_Tree *self, int IsBottom);
AB_node *AccessAB_node_child(AB_Tree *self, AB_node *node, int ith_child);


void* Ptr_ith_Key(AB_Tree *self, AB_node *node, int ith)
{
    return (void*) ((char*)node + sizeof(AB_node) + self->key_size * ith);
}

void *Ptr_ith_Child(AB_Tree *self, AB_node *node, int ith)
{
    size_t value_offset = sizeof(AB_node)+ (self->b-1) * self->key_size ;

    //return (void*) ((char*)node + value_offset + BIN_OPTION(node->IsBottom, self->value_size, sizeof(AB_node*)) * ith);

    //return (void*) ((char*)node + value_offset + (sizeof(AB_node*)*(node->IsBottom^1) + self->value_size*(node->IsBottom)) * ith);


    if (node->IsBottom == 0)
        return (void*) ((char*)node + value_offset + sizeof(AB_node*)* ith);
    else
        return (void*) ((char*)node + value_offset + self->value_size * ith);
}

void AB_node_MoveKeys(AB_Tree *self, node_arg dst_node, node_arg src_node, int moved_elem_cnt)
{
    void *dst = Ptr_ith_Key(self, dst_node.node, dst_node.ith_element); 
    void     *src = Ptr_ith_Key(self, src_node.node, src_node.ith_element);
    AB_node_datacpy(self, dst, src, self->key_size*moved_elem_cnt);
}


void AB_node_WriteKey(AB_Tree *self, AB_node *node, const void *key, int ith)
{
    void *dst = Ptr_ith_Key(self, node, ith);
    AB_node_datacpy(self, dst, key, self->key_size);
}

void AB_node_WriteChild(AB_Tree *self, AB_node *node, const void *child, int ith)
{
    void *dst = Ptr_ith_Child(self, node, ith);

    if (node->IsBottom)
        AB_node_datacpy(self, dst, child, self->value_size);
    else
        AB_node_datacpy(self, dst, child, sizeof(AB_node*));
}

void AB_node_MoveChildren(AB_Tree *self, node_arg dst_node, node_arg src_node, int moved_elem_cnt)
{
    void *dst = Ptr_ith_Child(self, dst_node.node, dst_node.ith_element), *src = Ptr_ith_Child(self, src_node.node, src_node.ith_element);

    if (src_node.node->IsBottom)
         AB_node_datacpy(self, dst, src, self->value_size*moved_elem_cnt);
    else
        AB_node_datacpy(self, dst, src, sizeof(AB_node*)*moved_elem_cnt);
}

void FreeAB_node(AB_Tree *self, AB_node *node)
{
    free((void*)node);
}


AB_node *RequestAB_node(AB_Tree *self, int IsBottom)
{



    AB_node *ret = NULL;

    //ret = (AB_node*)malloc(sizeof(AB_node) + self->key_size*(self->b-1) + (self->value_size*IsBottom +  sizeof(AB_node*)*(IsBottom^1))*self->b);

    if (IsBottom)
        ret = (AB_node*)malloc(sizeof(AB_node) + self->key_size*(self->b-1) + self->value_size*self->b);
    else
         ret = (AB_node*)malloc(sizeof(AB_node) + self->key_size*(self->b-1) + sizeof(AB_node*)*self->b);
    
    

    ret->IsBottom = IsBottom;
    ret->KeyCount = 0;

    if (ret == NULL)
        printf("malloc fail\n");

    return ret;

}
AB_node *AccessAB_node_child(AB_Tree *self, AB_node *node, int ith_child)
{
    AB_node *ret ;
    memcpy((void*)&ret, Ptr_ith_Child(self, node, ith_child), sizeof(AB_node*));
    return ret;

}



static void AB_node_datacpy(AB_Tree *self, void *dst, void *src, size_t byte_cnt)
{
    memcpy(dst, src, byte_cnt);
}