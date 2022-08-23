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
    int tmp[256];
    ab_tree = AB_tree_Map_init(9, 21, sizeof(int), sizeof(int [256]));
    srand(time(NULL));
    clock_t start, finish;
    double sum =0.0;
    int i = 0 ;
    int k,v;
    for(i = 10000000 ; i > 0 ; i--)
    {
        k = i;
        v = k*10;
        
        k = rand()%10000;
        k = 10000*k + rand()%10000;
        v = k%10;
        start = clock();
        if(i%6)
            ab_tree->Insert(ab_tree, &k, tmp);
        else
            ab_tree->Search(ab_tree, &k);
        //if (i%6 && ab_tree->Search(ab_tree, &k) == 0)
        //    printf("???\n"); 
        //ab_tree->GetValue(ab_tree, &v);
        finish = clock();

        sum = sum+((finish - start) / (double)CLOCKS_PER_SEC);

        if (i % 1000000 == 0)
        {
            _sleep(2000);
            srand(time(NULL));
        }
            

    }

    printf("%lf\n",sum);
    //GetTotalKeytCount(((AB_Tree_Map*)ab_tree)->TreeConfig);
    //tra(ab_tree->TreeConfig);
    GetTotalNodeCount(((AB_Tree_Map*)ab_tree)->TreeConfig);
    
    printf("\\\\\\\\\\\\\\\\\\\\\n");

    for(i = 10000000 ; i > 0 ; i--)
    {
        k = i;
        v = k*10;
        
        k = rand()%10000;
        k = 10000*k + rand()%10000;
        v = k%10;

        start = clock();
        ab_tree->Delete(ab_tree, &k);
        //ab_tree->Search(ab_tree, &k);

        //ab_tree->GetValue(ab_tree, &v);
        finish = clock();

        sum = sum+((finish - start) / (double)CLOCKS_PER_SEC);

        if (i % 1000000 == 0)
        {
            _sleep(2000);
            srand(time(NULL));
        }
    

    }

    printf("%lf\n",sum);
    return 0;

}

