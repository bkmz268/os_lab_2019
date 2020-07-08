CC=gcc
CFLAGS=-I.

all : factorial.c server.c client.c
	gcc -c factorial.c -o factorial.o
	gcc -shared factorial.o -o libfactorial.so
	gcc server.c -L. -lfactorial -lpthread -o server
	gcc client.c -L. -lfactorial -lpthread -o client

clean :
	rm factorial.o libfactorial.so server client
