INCLUDE = ../


all:output1	output2	output3 output4

output1:test.o $(wildcard $(INCLUDE)*.o)
		gcc -g $^ -o test

output2:test_copy.o $(wildcard $(INCLUDE)*.o)
		gcc -g $^ -o test_copy

output3:test_copy2.o $(wildcard $(INCLUDE)*.o)
		gcc -g $^ -o test_copy2

output4:test_copy3.o $(wildcard $(INCLUDE)*.o)
	gcc -g $^ -o test_copy3



test.o:test.c $(INCLUDE)AB_tree_map.h	
	gcc -g -c -I $(INCLUDE) test.c	

test_copy.o:test_copy.c $(INCLUDE)AB_tree_map.h	
	gcc -g -c -I $(INCLUDE) test_copy.c	

test_copy2.o:test_copy2.c $(INCLUDE)$\
	AB_tree_map.h	
	gcc -g -c -I $(INCLUDE) test_copy2.c	

test_copy3.o:test_copy3.c $(INCLUDE)AB_tree_map.h	
	gcc -g -c -I $(INCLUDE) test_copy3.c	

clean:
	del *exe *o

TestAll:
	./test
	./test_copy
	./test_copy2
	./test_copy3