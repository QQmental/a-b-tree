#ifndef NODE_ARG_H
#define NODE_ARG_H
#include "AB_node.h"

struct node_arg
{
    AB_node *node;
    int ith_element;
};
typedef struct node_arg node_arg;
#endif