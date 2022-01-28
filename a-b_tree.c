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
        if (cmp < 0)
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
static inline AB_internel_node *MakeNode(AB_Tree *self, size_t child_size)
{
    AB_internel_node *new_node;
    new_node = (AB_internel_node *)malloc(sizeof(AB_internel_node)+ (self->b-1) * self->key_size);
    new_node->children = malloc(child_size*(self->b));
    return new_node;
}


static inline AB_internel_node *SplitNode(AB_Tree *self, AB_internel_node *node, int new_key_pos)
{

    size_t child_size ;

    if (node->IsBottom)
        child_size = self->value_size;
    else
        child_size = sizeof(AB_internel_node*);

    AB_internel_node *new_node ;
    new_node = MakeNode(self, child_size);
    new_node->IsBottom = node->IsBottom;
    node->KeyConut = ((node->KeyConut+1) / 2);
    new_node->KeyConut = self->b - node->KeyConut;

    if (new_key_pos < node->KeyConut) //new key belongs to old_node
    {

        /*之後拷貝部分key 和 children過去*/
        memcpy(Ptr_ith_Key(self, new_node, 0),
                Ptr_ith_Key(self, node, node->KeyConut-1),
                self->key_size * new_node->KeyConut);

        memcpy(Ptr_ith_Child(self, new_node, 0),
                Ptr_ith_Child(self, node, node->KeyConut-1),
                child_size * (new_node->KeyConut+1));

        memcpy(Ptr_ith_Key(self, node, new_key_pos+1),
                Ptr_ith_Key(self, node, new_key_pos),
                self->key_size * (node->KeyConut - new_key_pos));

        memcpy(Ptr_ith_Child(self, node, new_key_pos+1),
                Ptr_ith_Child(self, node, new_key_pos),
                child_size * (node->KeyConut - new_key_pos +1));
    }

    else//key belongs to new_node
    {
        memcpy(Ptr_ith_Key(self, new_node, 0),
                Ptr_ith_Key(self, node, node->KeyConut),
                self->key_size * (new_key_pos - node->KeyConut));

        memcpy(Ptr_ith_Key(self, new_node, new_key_pos - node->KeyConut + 1),
                Ptr_ith_Key(self, node, new_key_pos),
                self->key_size * (node->KeyConut + new_node->KeyConut - new_key_pos - 1));

        memcpy(Ptr_ith_Child(self, new_node, 0),
                Ptr_ith_Child(self, node, node->KeyConut),
                child_size *(new_key_pos - node->KeyConut));

        memcpy(Ptr_ith_Child(self, new_node, new_key_pos - node->KeyConut + 1),
                Ptr_ith_Child(self, node, new_key_pos),
                child_size * (node->KeyConut + new_node->KeyConut - new_key_pos));

    }
    return new_node;
}


static inline AB_internel_node *AddData(AB_Tree *self, AB_internel_node *node, KV_pair kv, int new_key_pos)
{

    size_t child_size ;

    if (node->IsBottom)
        child_size = self->value_size;
    else
        child_size = sizeof(AB_internel_node*);


    if (IsFullNode(self, node))
    {
        AB_internel_node *new_node ;
        new_node = SplitNode(self, node, new_key_pos);
        if (new_key_pos < node->KeyConut)
        {
            memcpy(Ptr_ith_Key(self, node, new_key_pos),
                    kv.key,
                    self->key_size);
            memcpy(Ptr_ith_Child(self, node, new_key_pos),
                    kv.value,
                    child_size);
        }
        else
        {
            memcpy(Ptr_ith_Key(self, new_node, new_key_pos - node->KeyConut),
                    kv.key,
                    self->key_size);
            memcpy(Ptr_ith_Child(self, new_node, new_key_pos - node->KeyConut),
                    kv.value,
                    child_size);
        }
        return new_node;
    }
    else
    {
        //平移插入key

        memcpy(Ptr_ith_Key(self, node, new_key_pos+1),
                Ptr_ith_Key(self, node, new_key_pos),
                self->key_size * (node->KeyConut-new_key_pos));

        memcpy(Ptr_ith_Key(self, node, new_key_pos),
                kv.key,
                self->key_size);


        memcpy(Ptr_ith_Child(self, node, new_key_pos+1),
                Ptr_ith_Child(self, node, new_key_pos),
                child_size * ((node->KeyConut-new_key_pos+1)));

        memcpy(Ptr_ith_Child(self, node, new_key_pos),
                kv.value,
                child_size);

        node->KeyConut += 1;
        return NULL;
    }
}



void AB_tree_Insert_algo(AB_Tree *self,  void *key, void *value)
{
    AB_internel_node *node = self->root;
    self->SearchPathLength = 0;
    self->KeyIsFound = 0;
    int rank;

    while (1)
    {
        rank = GetKeyRank(self, node, key);
        self->SearchPath[self->SearchPathLength] = node;
        self->SearchBranch[self->SearchPathLength] = rank;
        self->SearchPathLength++;
        if (node->IsBottom)
            break;
        node = *(AB_internel_node**)Ptr_ith_Child(self, node, rank);
    }
    //rank = GetKeyRank(self, node, key);

    if (!self->KeyIsFound)
    {
        node = AddData(self, node, KV(key, value), rank);
        if (node == NULL)
            return;

        int level = self->SearchPathLength-1;

        while(level > 0)
        {
            rank = self->SearchBranch[level-1];
            key = *(void**)Ptr_ith_Key(self,
                                        self->SearchPath[level],
                                        self->SearchPath[level]->KeyConut-1);

            node = AddData(self, self->SearchPath[level-1], KV(key, node), rank);
            self->SearchPath[level-1]->KeyConut -= 1;

            if (node == NULL)
                break;
            level -= 1;
        }
        if (level == 0 && node)
        {

            AB_internel_node* tmp_root = self->root;
            self->root = MakeNode(self, sizeof(AB_internel_node*));
            //self->root = (AB_internel_node *)malloc(sizeof(AB_internel_node)+ (self->b-1) * self->key_size);
            //self->root->children = malloc(sizeof(AB_internel_node*)*(self->b));

            self->root->IsBottom = 0 ;
            self->root->KeyConut = 0 ;

            memcpy(Ptr_ith_Key(self, self->root, 0),
                    Ptr_ith_Key(self, tmp_root, tmp_root->KeyConut-1),
                    self->key_size);
            tmp_root->KeyConut -= 1;

            memcpy(Ptr_ith_Child(self, self->root, 0),
                    (void**)&tmp_root,
                    sizeof(AB_internel_node*));

            memcpy(Ptr_ith_Child(self, self->root, 1),
                    (void**)&node,
                    sizeof(AB_internel_node*));

            self->root->KeyConut = 1;
        }
    }
    else
    {
        memcpy(Ptr_ith_Child(self, node, rank), value, self->value_size);
    }
}

void tra(AB_Tree *self)
{
    self->SearchPathLength = 1 ;
    self->SearchPath[0] = self->root;
    while(self->SearchPathLength > 0)
    {
        if (self->SearchPath[self->SearchPathLength-1]->IsBottom)
        {
            int i = 0;
            for(; i < self->SearchPath[self->SearchPathLength-1]->KeyConut ; i++)
            {
                printf("%d ", *(int*)Ptr_ith_Key(self, self->SearchPath[self->SearchPathLength-1], i));
            }
            printf("\n");
            self->SearchPathLength -= 1;
            continue;
        }
        if (self->SearchBranch[self->SearchPathLength-1]
            == self->SearchPath[self->SearchPathLength-1]->KeyConut)
        {
            self->SearchPathLength -= 1;
            continue;
        }
        int branch = self->SearchBranch[self->SearchPathLength-1];
        self->SearchPath[self->SearchPathLength]
        = Ptr_ith_Child(self, self->SearchPath[self->SearchPathLength-1], branch);
        self->SearchBranch[self->SearchPathLength-1] += 1;
        self->SearchPathLength++;
    }



}



int main()
{

   /*
    AB_Tree ab_tree = {.a=3, .b=7, .key_size = sizeof(int), .value_size = sizeof(int), .SearchPathLength = 0, .KeyComp = Comp};
    ab_tree.root = (AB_internel_node*)malloc(sizeof(AB_internel_node)+ab_tree.b*ab_tree.key_size);
    ab_tree.root->IsBottom = 1;
    ab_tree.root->children = malloc((ab_tree.b+1)*ab_tree.value_size);

    ab_tree.root->KeyConut = 6;
    *(int*)Ptr_ith_Key(&ab_tree, ab_tree.root, 0) = 100;
    *(int*)Ptr_ith_Key(&ab_tree, ab_tree.root, 1) = 200;
    *(int*)Ptr_ith_Key(&ab_tree, ab_tree.root, 2) = 300;
    *(int*)Ptr_ith_Key(&ab_tree, ab_tree.root, 3) = 400;
    *(int*)Ptr_ith_Key(&ab_tree, ab_tree.root, 4) = 500;
    *(int*)Ptr_ith_Key(&ab_tree, ab_tree.root, 5) = 600;

    *(int*)Ptr_ith_Child(&ab_tree, ab_tree.root, 0) = 10;
    *(int*)Ptr_ith_Child(&ab_tree, ab_tree.root, 1) = 20;
    *(int*)Ptr_ith_Child(&ab_tree, ab_tree.root, 2) = 30;
    *(int*)Ptr_ith_Child(&ab_tree, ab_tree.root, 3) = 40;
    *(int*)Ptr_ith_Child(&ab_tree, ab_tree.root, 4) = 50;
    *(int*)Ptr_ith_Child(&ab_tree, ab_tree.root, 5) = 60;
    *(int*)Ptr_ith_Child(&ab_tree, ab_tree.root, 6) = 0x7FFFFFFF;

    int key = 350, value = 100;
    AB_internel_node *new_node = AddData(&ab_tree, ab_tree.root, KV(&key, &value), 3);
    int i ;
    for(i = 0 ; i < ab_tree.root->KeyConut ; i++)
    {
        printf("%d ",*(int*)Ptr_ith_Key(&ab_tree, ab_tree.root, i));
    }
    printf("\n");
    for(i = 0 ; i < ab_tree.root->KeyConut+1 ; i++)
    {
        printf("%d ",*(int*)Ptr_ith_Child(&ab_tree, ab_tree.root, i));
    }
    printf("\n");
    printf("new_node\n");
    for(i = 0 ; i < new_node->KeyConut ; i++)
    {
        printf("%d ",*(int*)Ptr_ith_Key(&ab_tree, new_node, i));
    }
    printf("\n");
    for(i = 0 ; i < new_node->KeyConut+1 ; i++)
    {
        printf("%d ",*(int*)Ptr_ith_Child(&ab_tree, new_node, i));
    }
    printf("\n");

    */

    AB_Tree ab_tree = {.a=3, .b=6, .key_size = sizeof(int), .value_size = sizeof(int), .SearchPathLength = 0, .KeyComp = Comp};
    //ab_tree.root = (AB_internel_node*)malloc(sizeof(AB_internel_node)+(ab_tree.b-1)*ab_tree.key_size);
    //ab_tree.root->children = malloc((ab_tree.b+2)*ab_tree.value_size);

    ab_tree.root = MakeNode(&ab_tree, ab_tree.value_size);
    ab_tree.root->IsBottom = 1;
    ab_tree.root->KeyConut = 0;


    int key = 10, value = 100;
    AB_tree_Insert_algo(&ab_tree, &key, &value);
    key = 20, value = 200;
    AB_tree_Insert_algo(&ab_tree, &key, &value);
    key = 30, value = 300;
    AB_tree_Insert_algo(&ab_tree, &key, &value);
    key = 40, value = 400;
    AB_tree_Insert_algo(&ab_tree, &key, &value);
    key = 50, value = 500;
    AB_tree_Insert_algo(&ab_tree, &key, &value);
    key = 60, value = 600;
    AB_tree_Insert_algo(&ab_tree, &key, &value);
    key = 70, value = 700;
    AB_tree_Insert_algo(&ab_tree, &key, &value);
    key = 55, value = 550;
    AB_tree_Insert_algo(&ab_tree, &key, &value);
    printf("bottom : %d\n",ab_tree.root->IsBottom);
    key = 65, value = 650;
    AB_tree_Insert_algo(&ab_tree, &key, &value);
    //key = 75, value = 750;
    //AB_tree_Insert_algo(&ab_tree, &key, &value);
    //key = 85, value = 850;
    //AB_tree_Insert_algo(&ab_tree, &key, &value);
    /*srand(time(NULL));
    clock_t start, finish;
    double sum =0.0;
    int a = 300;
    for(a = 1 ; a <= 10 ; a++)
    {
        int x = rand()%10000;
        int y = rand()%10000;
        x = x*10000+y;
        start = clock();
        AB_tree_Insert_algo(&ab_tree, &x, &y);
        finish = clock();
        sum = sum+((finish - start) / (double)CLOCKS_PER_SEC);
        if (a%1000000 == 0)
            srand(time(NULL));
    }
    printf("%lf\n",sum);*/
    //tra(&ab_tree);

    //return 0;
    printf("root %d\n",ab_tree.root->IsBottom);
    int i ;
    for(i = 0 ; i < ab_tree.root->KeyConut ; i++)
    {
        printf("%d ", *(int*)Ptr_ith_Key(&ab_tree, ab_tree.root, i));
    }
    printf("\n");

    //return 0 ;

    printf("fst\n");
    AB_internel_node *node = *(AB_internel_node **)Ptr_ith_Child(&ab_tree, ab_tree.root, 0);
    for(i = 0 ; i < node->KeyConut ; i++)
    {
        printf("%d ", *(int*)Ptr_ith_Key(&ab_tree, node, i));
    }

    printf("\n");

    for(i = 0 ; i < node->KeyConut+1 ; i++)
    {
        printf("%d ",*(int*)Ptr_ith_Child(&ab_tree, node, i));
    }
    printf("\n");

    node = *(AB_internel_node **)Ptr_ith_Child(&ab_tree, ab_tree.root, 1);
    printf("snd\n");
    for(i = 0 ; i < node->KeyConut ; i++)
    {
        printf("%d ", *(int*)Ptr_ith_Key(&ab_tree, node, i));
    }

    printf("\n");
    for(i = 0 ; i < node->KeyConut+1 ; i++)
    {
        printf("%d ",*(int*)Ptr_ith_Child(&ab_tree, node, i));
    }
    printf("\n");

    printf("hihi 1\n");
    printf("hihi 2\n");
    printf("hihi 3\n");
    printf("hihi 4\n");
    printf("hihi 5\n");
    printf("hihi 6\n");
}
