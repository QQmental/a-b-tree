#include "AB_KV_util.h"
#include "AB_node_acc_mod.h"
void AB_key_dtor(AB_Tree *self, AB_node *node, int ith_key);
void AB_child_dtor(AB_Tree *self, AB_node *node, int ith_child);
int AB_CompareKey(AB_Tree *self, const void *key, AB_node *node, int ith_key);



void AB_key_dtor(AB_Tree *self, AB_node *node, int ith_key)
{
    void *dst = Ptr_ith_Key(self, node, ith_key);
    self->KeyDestroy(dst);
}

void AB_child_dtor(AB_Tree *self, AB_node *node, int ith_child)
{
    void *dst = Ptr_ith_Child(self, node, ith_child);
    self->ValueDestroy(dst);
}

int AB_CompareKey(AB_Tree *self, const void *key, AB_node *node, int ith_key)
{ 
    void *dst = Ptr_ith_Key(self, node, ith_key);

    return self->KeyComp(key, dst);
}