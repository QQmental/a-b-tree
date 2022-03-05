#ifndef AB_KV_UTIL_H
#define AB_KV_UTIL
#include "AB_tree_algo.h"

void AB_key_dtor(AB_Tree *self, AB_node *node, int ith_key);
void AB_child_dtor(AB_Tree *self, AB_node *node, int ith_child);
int AB_CompareKey(AB_Tree *self, void *key, AB_node *node, int ith_key);

#endif