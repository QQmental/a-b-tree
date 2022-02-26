#include"AB_node_acc_mod.h"
static void AB_node_datacpy(AB_Tree *self, void *dst, void *src, size_t byte_cnt);
void AB_node_WriteKey(AB_Tree *self, AB_internel_node *node, const void *key, int ith);
void AB_node_WriteChild(AB_Tree *self, AB_internel_node *node, const void *child, int ith);
void AB_node_MoveKeys(AB_Tree *self, AB_internel_node *dst_node, AB_internel_node *src_node, int dst_ith_key, int src_ith_key, int moved_elem_cnt);
void AB_node_MoveChildren(AB_Tree *self, AB_internel_node *dst_node, AB_internel_node *src_node, int dst_ith_child, int src_ith_child, int moved_elem_cnt);





void AB_node_WriteKey(AB_Tree *self, AB_internel_node *node, const void *key, int ith)
{
    void *dst = Ptr_ith_Key(self, node, ith);
    AB_node_datacpy(self, dst, key, self->key_size);
}

void AB_node_WriteChild(AB_Tree *self, AB_internel_node *node, const void *child, int ith)
{
    void *dst = Ptr_ith_Child(self, node, ith);

    if (node->IsBottom)
        AB_node_datacpy(self, dst, child, self->value_size);
    else
        AB_node_datacpy(self, dst, child, sizeof(AB_internel_node*));
}

void AB_node_MoveKeys(AB_Tree *self, AB_internel_node *dst_node, AB_internel_node *src_node, int dst_ith_key, int src_ith_key, int moved_elem_cnt)
{
    void *dst = Ptr_ith_Key(self, dst_node, dst_ith_key), *src = Ptr_ith_Key(self, src_node, src_ith_key);
    AB_node_datacpy(self, dst, src, self->key_size*moved_elem_cnt);
}

void AB_node_MoveChildren(AB_Tree *self, AB_internel_node *dst_node, AB_internel_node *src_node, int dst_ith_child, int src_ith_child, int moved_elem_cnt)
{
    void *dst = Ptr_ith_Child(self, dst_node, dst_ith_child), *src = Ptr_ith_Child(self, src_node, src_ith_child);

    if (src_node->IsBottom)
         AB_node_datacpy(self, dst, src, self->value_size*moved_elem_cnt);
    else
        AB_node_datacpy(self, dst, src, sizeof(AB_internel_node*)*moved_elem_cnt);

}

static void AB_node_datacpy(AB_Tree *self, void *dst, void *src, size_t byte_cnt)
{
    memcpy(dst, src, byte_cnt);
}