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
    printf("Hello world!!!!\n");
    KV_map *ab_tree;
    ab_tree = AB_tree_Map_init(4, 10, sizeof(int), sizeof(int));
    srand(time(NULL));
    clock_t start, finish;
    double sum =0.0;
    int i = 0 ;
    int k,v;
    for(i = 0 ; i < 10000000 ; i++)
    {
        k = i;
        v = k*10;
        //k = rand()%10000;
        //k = 10000*k + rand()%10000;
        v = k%10;


        start = clock();
        ab_tree->Insert(ab_tree, &k, &v);
        finish = clock();
        sum = sum+((finish - start) / (double)CLOCKS_PER_SEC);
        if (i%1000000 == 0)
        {
            printf("time %lf\n",sum);
            srand(time(NULL));
        }   
        //printf("insert <%d,%d>\n",k,v);
    }
    srand(time(NULL));
    printf("%lf\n",sum);
    printf("deletion\n");
    for(i = 0 ; i < 10000000; i++)
    {
        k = i;
        v = k*10;
        //k = rand()%10000;
       // k = 10000*k + rand()%10000;
        start = clock();
        ab_tree->Delete(ab_tree, &k);
        
        //printf("delete %d\n",k);
        
        finish = clock();
        if (ab_tree->Search(ab_tree, &k))
            printf("??? %d\n",k);

        if (ab_tree->Delete(ab_tree, &k) == 1)
            printf("del ??? %d\n",k);
        //printf("delete %d\n",k);
        
        sum = sum+((finish - start) / (double)CLOCKS_PER_SEC);
        if (i%1000000 == 0)
        {
            printf("time %lf\n",sum);
            srand(time(NULL));
        }
            

           
        //printf("insert <%d,%d>\n",k,v);
    }
    printf("%lf\n",sum);
    clea(ab_tree);
    //tra(ab_tree.TreeConfig);
    return 0;
    /*for(i = 0 ; i < 10000000 ; i++)
    {
        k = i;
        v = k*10;
        start = clock();
        //ab_tree.Search(&ab_tree, &k);
        ab_tree.Delete(&ab_tree, &k);
        finish = clock();
        sum = sum+((finish - start) / (double)CLOCKS_PER_SEC);

        //printf("insert <%d,%d>\n",k,v);
    }*/
    //printf("%lf\n",sum);
    //ab_tree.Delete(&ab_tree, &k);
   
    
    return 0;
}

