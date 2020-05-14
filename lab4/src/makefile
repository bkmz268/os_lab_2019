CC=gcc
CFLAGS=-I.


all : process_memory parallel_min_max parallel_sum zombie

parallel_min_max : utils.o find_min_max.o utils.h find_min_max.h parallel_min_max.o
	$(CC) -o parallel_min_max utils.o find_min_max.o parallel_min_max.o

process_memory : process_memory.c 
	$(CC) -o process_memory process_memory.c

parallel_sum : utils.o utils.h parallel_sum.o
	$(CC) -o parallel_sum -lpthread utils.o parallel_sum.o

zombie : 
	$(CC) -o zombie zombie.c $(CFLAGS)

utils.o : utils.h
	$(CC) -o utils.o -c -std=c99 utils.c $(CFLAGS)

find_min_max.o : utils.h find_min_max.h
	$(CC) -o find_min_max.o -c -std=c99 find_min_max.c $(CFLAGS)

parallel_min_max.o :  utils.h find_min_max.h
	$(CC) -o parallel_min_max.o -c -std=c99 parallel_min_max.c $(CFLAGS)

parallel_sum.o : utils.h
	$(CC) -o parallel_sum.o -c -std=c99 parallel_sum.c $(CFLAGS)

clean :
	rm utils.o find_min_max.o parallel_min_max.o parallel_sum.o parallel_min_max process_memory parallel_sum zombie