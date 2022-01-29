#include <stdio.h>
#include <stdlib.h>
#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include <time.h>
struct KV_pair
{
    void *key, *value;
};
typedef struct KV_pair KV_pair;

struct AB_internel_node
{
    void *children;
    short KeyConut;
    char IsBottom ;

};
typedef struct AB_internel_node AB_internel_node ;
struct AB_Tree
{
    AB_internel_node *root;
    AB_internel_node *SearchPath[32];
    short SearchBranch[32];
    void *privae_cache;
    int (*KeyComp)(void*, void*);
    size_t key_size, value_size;
    int SearchPathLength ;
    int a, b;
    int KeyIsFound;

};

typedef struct AB_Tree AB_Tree;

KV_pair KV(void *k, void *v)
{
    KV_pair ret = {.key = k, .value = v};
    return ret;
}

static int Comp(void *a, void *b)
{
    if (*(int*)a > *(int*)b)
        return 1 ;

    if (*(int*)a < *(int*)b)
        return -1 ;

    if (*(int*)a == *(int*)b)
        return 0 ;

}
static inline void* Ptr_ith_Key(AB_Tree *self, AB_internel_node *node, int ith)
{
    return (void*) ((char*)node + sizeof(AB_internel_node) + self->key_size * ith);
}
static inline int GetKeyRank(AB_Tree *self, AB_internel_node *node, void *key)
{
    int i = 0 ;
    for(; i < node->KeyConut ; i++)
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
}
static inline int IsFullNode(AB_Tree *self, AB_internel_node *node)
{
    return node->KeyConut == self->b-1;
}
static inline void *Ptr_ith_Child(AB_Tree *self, AB_internel_node *node, int ith)
{
    if (node->IsBottom == 0)
        return (void*) ((char*)node->children +  sizeof(AB_internel_node*)* ith);
    else
        return (void*) ((char*)node->children + self->value_size * ith);
}
static inline AB_internel_node *MakeNode(AB_Tree *self, size_t child_size, int IsBottom)
{
    AB_internel_node *new_node;
    new_node = (AB_internel_node *)malloc(sizeof(AB_internel_node)+ (self->b-1) * self->key_size);
    new_node->children = malloc(child_size*(self->b));
    new_node->IsBottom = IsBottom;
    return new_node;
}


static inline void ShifKeys(AB_Tree *self, AB_internel_node *node, int pos)
{
    memcpy(Ptr_ith_Key(self, node, pos+1),
           Ptr_ith_Key(self, node, pos),
           self->key_size * (node->KeyConut-pos));
}

static inline void ShifChildren(AB_Tree *self, AB_internel_node *node, int pos, size_t child_size)
{
    memcpy(Ptr_ith_Child(self, node, pos+1),
           Ptr_ith_Child(self, node, pos),
           child_size * (node->KeyConut-pos+1));
}

void AB_tree_Search_algo(AB_Tree *self, void *key)
{
    self->KeyIsFound = 0;
    self->SearchPathLength = 0;
    AB_internel_node *node = self->root;
    while(1)
    {
        int rank = GetKeyRank(self, node, key);
        self->SearchBranch[self->SearchPathLength] = rank;
        self->SearchPath[self->SearchPathLength] = node;
        self->SearchPathLength++;
        if (node->IsBottom)
            break;

        node = *(AB_internel_node**)Ptr_ith_Child(self, node, rank);
    }
}


AB_internel_node *SplitNode(AB_Tree *self, AB_internel_node *cur, KV_pair kv, int new_key_pos)
{
    size_t child_size = sizeof(AB_internel_node *);

    if (cur->IsBottom)
        child_size = self->value_size ;

    AB_internel_node *new_node = MakeNode(self, child_size, cur->IsBottom);
    cur->KeyConut = (self->b)/2;
    new_node->KeyConut = self->b - cur->KeyConut;

    if (new_key_pos < (self->b)/2) //new key resides in old node
    {
        memcpy(Ptr_ith_Key(self, new_node, 0),
               Ptr_ith_Key(self, cur, cur->KeyConut-1),
               self->key_size * new_node->KeyConut);
        memcpy(Ptr_ith_Child(self, new_node, 0),
               Ptr_ith_Child(self, cur, cur->KeyConut-1),
               child_size * (new_node->KeyConut + 1));

        ShifKeys(self, cur, new_key_pos);
        ShifChildren(self, cur, new_key_pos, child_size);
        memcpy(Ptr_ith_Key(self, cur, new_key_pos),
               kv.key,
               self->key_size);
        memcpy(Ptr_ith_Child(self, cur, new_key_pos),
               kv.value,
               child_size);
    }
    else //new key resides in new node
    {
        memcpy(Ptr_ith_Key(self, new_node, 0),
                Ptr_ith_Key(self, cur, cur->KeyConut),
                self->key_size * (new_key_pos - cur->KeyConut));

        memcpy(Ptr_ith_Key(self, new_node, new_key_pos - cur->KeyConut + 1),
                Ptr_ith_Key(self, cur, new_key_pos),
                self->key_size * (cur->KeyConut + new_node->KeyConut - new_key_pos - 1));

        memcpy(Ptr_ith_Child(self, new_node, 0),
                Ptr_ith_Child(self, cur, cur->KeyConut),
                child_size *(new_key_pos - cur->KeyConut));

        memcpy(Ptr_ith_Child(self, new_node, new_key_pos - cur->KeyConut + 1),
                Ptr_ith_Child(self, cur, new_key_pos),
                child_size * (cur->KeyConut + new_node->KeyConut - new_key_pos));

        memcpy(Ptr_ith_Key(self, new_node, new_key_pos-cur->KeyConut),
               kv.key,
               self->key_size);
        memcpy(Ptr_ith_Child(self, new_node, new_key_pos-cur->KeyConut),
               kv.value,
               child_size);
    }

    return new_node ;
}

AB_internel_node *AddData(AB_Tree *self, AB_internel_node *cur, KV_pair kv, int new_key_pos)
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
        ShifKeys(self, cur, new_key_pos);
        ShifChildren(self, cur, new_key_pos, child_size);

        memcpy(Ptr_ith_Key(self, cur, new_key_pos),
               kv.key,
               self->key_size);
        memcpy(Ptr_ith_Child(self, cur, new_key_pos),
               kv.value,
               child_size);
        cur->KeyConut += 1;
        return NULL;
    }
}




void AB_tree_Insert_algo(AB_Tree *self, KV_pair kv)
{
    AB_tree_Search_algo(self, kv.key);

    int level = self->SearchPathLength-1;

    AB_internel_node *cur = self->SearchPath[level], *new_node;

    if (self->KeyIsFound)
    {
        memcpy(Ptr_ith_Child(self, cur, self->SearchBranch[level]),
               kv.value,
               self->value_size);
        return ;
    }

    AB_internel_node *chd;
    while(1)
    {
        new_node = AddData(self, cur, kv, self->SearchBranch[level]);
        level -= 1;

        if (new_node == NULL || level == -1)
            break;
        kv.key = Ptr_ith_Key(self, self->SearchPath[level+1], self->SearchPath[level+1]->KeyConut-1);
        self->SearchPath[level+1]->KeyConut -= 1;
        chd = cur;
        kv.value = (void*)&chd;

        cur = self->SearchPath[level];

        memcpy(Ptr_ith_Child(self, cur, self->SearchBranch[level]),(void*)&new_node, sizeof(AB_internel_node*));
    }

    if (level == -1 && new_node)// the root was full and it splits
    {
        AB_internel_node *tmp_root = self->root;
        self->root = MakeNode(self, sizeof(AB_internel_node*), 0);
        self->root->KeyConut = 1;
        memcpy(Ptr_ith_Key(self, self->root, 0),
               Ptr_ith_Key(self, tmp_root, tmp_root->KeyConut-1),
               self->key_size);

        memcpy(Ptr_ith_Child(self, self->root, 0),
               (void*)&tmp_root,
               sizeof(AB_internel_node*));

        memcpy(Ptr_ith_Child(self, self->root, 1),
               (void*)&new_node,
               sizeof(AB_internel_node*));
        tmp_root->KeyConut -= 1;
    }




}




void print_node(AB_Tree *self, AB_internel_node *node, char *name)
{
    int i ;
    printf("%s %d\n",name, node->KeyConut);
    for(i = 0 ; i < node->KeyConut ; i++)
        printf("%d ",*(int*)Ptr_ith_Key(self, node, i));
    printf("\n");
}
void tra(AB_Tree *self)
{
    self->SearchPathLength = 1 ;
    self->SearchPath[0] = self->root;
    //self->SearchBranch

}
int main()
{
    printf("Hello world!\n");

    AB_Tree ab_tree ;
    ab_tree.a = 3;
    ab_tree.b = 6;
    ab_tree.KeyComp = Comp;
    ab_tree.key_size = sizeof(int);
    ab_tree.value_size = sizeof(int);
    ab_tree.root = MakeNode(&ab_tree, ab_tree.value_size, 1);
    ab_tree.root->KeyConut = 0;
    int key = 10, value = 100;
    /*AB_tree_Insert_algo(&ab_tree, KV(&key, &value));
    key = 10, value = 100;
    AB_tree_Insert_algo(&ab_tree, KV(&key, &value));
    key = 20, value = 200;
    AB_tree_Insert_algo(&ab_tree, KV(&key, &value));
    key = 30, value = 300;
    AB_tree_Insert_algo(&ab_tree, KV(&key, &value));
    key = 40, value = 400;
    AB_tree_Insert_algo(&ab_tree, KV(&key, &value));
    key = 50, value = 500;
    AB_tree_Insert_algo(&ab_tree, KV(&key, &value));
    key = 60, value = 600;
    AB_tree_Insert_algo(&ab_tree, KV(&key, &value));
    key = 70, value = 700;
    AB_tree_Insert_algo(&ab_tree, KV(&key, &value));
    key = 80, value = 800;
    AB_tree_Insert_algo(&ab_tree, KV(&key, &value));
    key = 90, value = 900;
    AB_tree_Insert_algo(&ab_tree, KV(&key, &value));
    key = 100, value = 1000;
    AB_tree_Insert_algo(&ab_tree, KV(&key, &value));
    key = 110, value = 1100;
    AB_tree_Insert_algo(&ab_tree, KV(&key, &value));
    key = 110, value = 1100;
    AB_tree_Insert_algo(&ab_tree, KV(&key, &value));
    key = 120, value = 1100;
    AB_tree_Insert_algo(&ab_tree, KV(&key, &value));
    key = 130, value = 1100;
    AB_tree_Insert_algo(&ab_tree, KV(&key, &value));
    key = 140, value = 1100;
    AB_tree_Insert_algo(&ab_tree, KV(&key, &value));
    key = 150, value = 1100;
    AB_tree_Insert_algo(&ab_tree, KV(&key, &value));
    key = 160, value = 1100;
    AB_tree_Insert_algo(&ab_tree, KV(&key, &value));
    key = 170, value = 1100;
    AB_tree_Insert_algo(&ab_tree, KV(&key, &value));
    key = 180, value = 1100;
    AB_tree_Insert_algo(&ab_tree, KV(&key, &value));
    key = 190, value = 1100;
    AB_tree_Insert_algo(&ab_tree, KV(&key, &value));*/

    key = 4075, value = 1100;
    AB_tree_Insert_algo(&ab_tree, KV(&key, &value));
    key = 6033, value = 1100;
    AB_tree_Insert_algo(&ab_tree, KV(&key, &value));
    key = 5737, value = 1100;
    AB_tree_Insert_algo(&ab_tree, KV(&key, &value));
    key = 2805, value = 1100;
    AB_tree_Insert_algo(&ab_tree, KV(&key, &value));
    key = 267, value = 1100;
    AB_tree_Insert_algo(&ab_tree, KV(&key, &value));
    key = 2156, value = 1100;
    AB_tree_Insert_algo(&ab_tree, KV(&key, &value));
    key = 6630, value = 1100;
    AB_tree_Insert_algo(&ab_tree, KV(&key, &value));
    key = 9502, value = 1100;
    AB_tree_Insert_algo(&ab_tree, KV(&key, &value));
    key = 7569, value = 1100;
    AB_tree_Insert_algo(&ab_tree, KV(&key, &value));
    key = 6571, value = 1100;
    AB_tree_Insert_algo(&ab_tree, KV(&key, &value));
    key = 6572, value = 1100;
    AB_tree_Insert_algo(&ab_tree, KV(&key, &value));
    key = 6573, value = 1100;
    AB_tree_Insert_algo(&ab_tree, KV(&key, &value));
    key = 9999, value = 1100;
    AB_tree_Insert_algo(&ab_tree, KV(&key, &value));
/*
4075 75
6033 33
5737 37
2805 5
267 67
2156 56
6630 30
9502 2
7569 69
6571 71
*/
/*
    srand(time(NULL));
    clock_t start, finish;
    double sum =0.0;
    int a = 300;
    for(a = 1 ; a <= 30 ; a++)
    {
        int x = rand()%10000;
        int y = rand()%10000;
        x = x*10000+y;
        //printf("%d %d\n",x,y);
        start = clock();
        AB_tree_Insert_algo(&ab_tree, KV(&x, &y));
        finish = clock();
        sum = sum+((finish - start) / (double)CLOCKS_PER_SEC);
        if (a%1000000 == 0)
            srand(time(NULL));
    }
    printf("%lf\n",sum);
*/


    AB_internel_node *node = ab_tree.root;
    int i ;
    /*printf("root %d\n",node->KeyConut);
    for(i = 0 ; i < node->KeyConut ; i++)
        printf("%d ",*(int*)Ptr_ith_Key(&ab_tree, node, i));
    printf("\n");*/

    print_node(&ab_tree, ab_tree.root, "root");
    if (ab_tree.root->KeyConut)
        print_node(&ab_tree, *(AB_internel_node**)Ptr_ith_Child(&ab_tree, ab_tree.root, 0), "fst");
    if (ab_tree.root->KeyConut)
        print_node(&ab_tree, *(AB_internel_node**)Ptr_ith_Child(&ab_tree, ab_tree.root, 1), "snd");
    if (ab_tree.root->KeyConut > 1)
        print_node(&ab_tree, *(AB_internel_node**)Ptr_ith_Child(&ab_tree, ab_tree.root, 2), "3rd");
    if (ab_tree.root->KeyConut > 2)
        print_node(&ab_tree, *(AB_internel_node**)Ptr_ith_Child(&ab_tree, ab_tree.root, 3), "4th");

    /*printf("3rd\n");
    node = *(AB_internel_node**)Ptr_ith_Child(&ab_tree, ab_tree.root, 2);
    for(i = 0 ; i < node->KeyConut ; i++)
        printf("%d ",*(int*)Ptr_ith_Key(&ab_tree, node, i));
    printf("\n");*/
    return 0;
}
