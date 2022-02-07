#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include"AB_tree.h"

void print_int_int(const void *key, const void *value)
{
    printf("we found <%d %d>\n",*(int*)key, *(int*)value);
}

int main()
{
    printf("Hello world!\n");

    AB_Tree_Map ab_tree;
    AB_tree_init(&ab_tree, 2, 14, sizeof(int), sizeof(int));
    srand(time(NULL));
    clock_t start, finish;
    double sum =0.0;
    int i = 0 ;
    int k,v;
    for(i = 0 ; i < 10000000 ; i++)
    {
        k = i;
        v = k*10;
        start = clock();
        ab_tree.Insert(&ab_tree, &k, &v);
        finish = clock();
        sum = sum+((finish - start) / (double)CLOCKS_PER_SEC);

        //printf("insert <%d,%d>\n",k,v);
    }
    printf("%lf\n",sum);
    for(i = 0 ; i < 10000000 ; i++)
    {
        k = i;
        v = k*10;
        start = clock();
        ab_tree.Search(&ab_tree, &k);
        finish = clock();
        sum = sum+((finish - start) / (double)CLOCKS_PER_SEC);

        //printf("insert <%d,%d>\n",k,v);
    }
    printf("%lf\n",sum);
    /*v = 25;
    k = 2;
    printf("%d\n",ab_tree.Insert(&ab_tree, &k, &v));
    printf("insert <%d,%d>\n",k,v);
    v = 40;
    k = 200;
    printf("%d\n",ab_tree.Insert(&ab_tree, &k, &v));
    printf("insert <%d,%d>\n",k,v);
    v = 40;
    k = 4;
    printf("%d\n",ab_tree.Insert(&ab_tree, &k, &v));
    printf("insert <%d,%d>\n",k,v);
    printf("\n\n");
    k = 2;
    if(ab_tree.Search(&ab_tree, &k))
    {
        ab_tree.GetValue(&ab_tree, &v);
        printf("we found <%d %d>\n",k, v);
    }
    k = 8;
    if(ab_tree.Search(&ab_tree, &k))
    {
        ab_tree.GetValue(&ab_tree, &v);
        printf("we found <%d %d>\n",k, v);
    }
    k = 200;
    if(ab_tree.Search(&ab_tree, &k))
    {
        ab_tree.GetValue(&ab_tree, &v);
        printf("we found <%d %d>\n",k, v);
    }
    k = 10;
    if(ab_tree.Search(&ab_tree, &k))
    {
        ab_tree.GetValue(&ab_tree, &v);
        printf("we found <%d %d>\n",k, v);
    }
    k = 11;
    if(ab_tree.Search(&ab_tree, &k))
    {
        ab_tree.GetValue(&ab_tree, &v);
        printf("we found <%d %d>\n",k, v);
    }
    k = 13;
    if(ab_tree.Search(&ab_tree, &k))
    {
        ab_tree.GetValue(&ab_tree, &v);
        printf("we found <%d %d>\n",k, v);
    }
    k = 6;
    if(ab_tree.Search(&ab_tree, &k))
    {
        ab_tree.GetValue(&ab_tree, &v);
        printf("we found <%d %d>\n",k, v);
    }
    k = 9;
    if(ab_tree.Search(&ab_tree, &k))
    {
        ab_tree.GetValue(&ab_tree, &v);
        printf("we found <%d %d>\n",k, v);
    }
    k = 21;
    if(ab_tree.Search(&ab_tree, &k))
    {
        ab_tree.GetValue(&ab_tree, &v);
        printf("we found <%d %d>\n",k, v);
    }*/

    return 0;
}

