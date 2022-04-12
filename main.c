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
    KV_map *ab_tree;
    ab_tree = AB_tree_Map_init(6, 14, sizeof(int), sizeof(int));
    srand(time(NULL));
    clock_t start, finish;
    double sum =0.0;
    int i = 0 ;
    int k,v;
    for(i = 200000 ; i > 0 ; i--)
    {
        k = i;
        v = k*10;

        v = k%10;

        start = clock();
        ab_tree->Insert(ab_tree, &k, &v);
        if(ab_tree->Search(ab_tree, &k) == 0)
            printf("???\n"); 
        ab_tree->GetValue(ab_tree, &v);
        finish = clock();

        sum = sum+((finish - start) / (double)CLOCKS_PER_SEC);

    }

    printf("%lf\n",sum);
    //return 0;

    //tra(((AB_Tree_Map*)ab_tree)->TreeConfig);
    printf("\\\\\\\\\\\\\\\\\\\\\n");

    for(i = 2000000 ; i > 0 ; i--)
    {
        k = i;

        v = k-10;

        start = clock();
        ab_tree->Insert(ab_tree, &k, &v);
        if(ab_tree->Search(ab_tree, &k) == 0)
            printf("????DD?\n");
        ab_tree->GetValue(ab_tree, &v);
        if (v != k-10)
            printf("????AA\n");
        if (ab_tree->Delete(ab_tree, &k) == 0)
            printf("fail delete\n");
        finish = clock();
        //printf("%d %d\n",k, v);
        sum = sum+((finish - start) / (double)CLOCKS_PER_SEC);

    }

    return 0;
}

