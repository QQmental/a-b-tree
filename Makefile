xxx_o = AB_node_acc_mod.o
xxx_c = AB_node_acc_mod.c
xxx_h = AB_node_acc_mod.h
#xxx = 

output:main.c AB_tree.o AB_tree_algo.o $(xxx_o)
	gcc -g main.c AB_tree.o AB_tree_algo.o $(xxx_o) -o main

AB_tree.o:AB_tree.c AB_tree_map.h
	gcc -g -c AB_tree.c

AB_tree_algo.o:AB_tree_algo.c AB_tree_algo.h algo_helper.h 
	gcc -g -c AB_tree_algo.c

$(xxx_o):$(xxx_c) $(xxx_h)
	gcc -g -c $(xxx_c)

clean:
	del *.o main.exe

