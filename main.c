#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include"AB_tree_map.h"

void print_int_int(const void *key, const void *value)
{
    printf("we found <%d %d>\n",*(int*)key, *(int*)value);
}

int main()
{
    printf("Hello world!\n");
    AB_Tree_Map ab_tree;
    AB_tree_Map_init(&ab_tree, 2, 4, sizeof(int), sizeof(int));
    srand(time(NULL));
    clock_t start, finish;
    double sum =0.0;
    int i = 0 ;
    int k,v;
    for(i = 50 ; i > 0 ; i--)
    {
        k = i;
        v = k*10;
/*
        k = rand()%10000;
        k = 10000*k + rand()%10000;
        v = k%10;
*/


        ab_tree.Insert(&ab_tree, &k, &v);
        ab_tree.Search(&ab_tree, &k);
        ab_tree.GetValue(&ab_tree, &k);
        //printf("%d\n",v);

        sum = sum+((finish - start) / (double)CLOCKS_PER_SEC);
        
        //printf("insert <%d,%d>\n",k,v);
    }

    tra(ab_tree.TreeConfig);
    return 0;
}

