CC = cc
CFLAGS = -Wall -g -D__DEBUG__

all: sol_dl_list.o sol_hash.o sol_set.o sol_stack.o sol_utils.o sol_list.o \
	sol_rbtree.o sol_rbtree_iter.o

sol_dl_list.o: sol_dl_list.c sol_common.h
sol_list.o: sol_list.c sol_common.h
sol_hash.o: sol_hash.c sol_common.h
sol_set.o: sol_set.c sol_hash.o sol_common.h
sol_stack.o: sol_stack.c sol_dl_list.o sol_common.h
sol_utils.o: sol_utils.c
sol_rbtree.o: sol_rbtree.c sol_common.h
sol_rbtree_iter.o: sol_rbtree_iter.c sol_rbtree.o sol_stack.o sol_dl_list.o sol_common.h

test_hash: test_hash.c sol_hash.o Hash_fnv.c  Hash_murmur.c
test_set: test_set.c sol_set.o sol_hash.o Hash_fnv.c  Hash_murmur.c
test_dl_list: test_dl_list.c sol_dl_list.o
test_list: test_list.c sol_list.o
test_stack: test_stack.c sol_stack.o sol_dl_list.o
test_rbtree: test_rbtree.c sol_rbtree.o sol_rbtree_iter.o sol_stack.o sol_dl_list.o

.PHONY: clean
clean:
	-rm -rf output *.o *.gch test_hash test_set test_dl_list test_stack test_list test_rbtree
