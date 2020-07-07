CC=gcc
CFLAGS=-I.

all : factor.c server.c client.c
	gcc -c factor.c -o factor.o
	gcc -shared factor.o -o libfactor.so
	gcc server.c -L. -lfactor -lpthread -o server
	gcc client.c -L. -lfactor -lpthread -o client

clean :
	rm factor.o libfactor.so server client
