xxx_o = AB_node_acc_mod.o
xxx_c = AB_node_acc_mod.c
xxx_h = AB_node_acc_mod.h
#xxx = 

OBJS = AB_tree.o AB_tree_algo.o $(xxx_o) AB_KV_util.o

output:main.c AB_tree.o AB_tree_algo.o AB_node_acc_mod.o AB_KV_util.o
		gcc -g $^ -o main

AB_tree.o:AB_tree.c AB_tree_map.h AB_tree_algo.h AB_node_acc_mod.h
	gcc -g -c -O2 AB_tree.c

AB_tree_algo.o:AB_tree_algo.c AB_tree_algo.h algo_helper.h 
	gcc -g -c -O2 AB_tree_algo.c

AB_KV_util.o:AB_KV_util.c AB_KV_util.h AB_node_acc_mod.h
	gcc -g -c -O2 AB_KV_util.c

AB_node_acc_mod.o:AB_node_acc_mod.c AB_node_acc_mod.h
	gcc -g -c -O2 AB_node_acc_mod.c	

clean:
	del *.o main.exe

