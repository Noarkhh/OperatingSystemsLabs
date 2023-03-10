CC=gcc
CFLAGS=-Wall

all: reverse1024 reverse1

reverse1024:
	$(CC) $(CFLAGS) $(TESTING) -D BLOCK_SIZE=1024 -o reverse1024 reverse.c

reverse1:
	$(CC) $(CFLAGS) $(TESTING) -o reverse1 reverse.c

clean:
	rm *a *o reverse1 reverse1024

test:
	rm -f pomiar_zad_2.txt
	make all TESTING="-D TESTING"
	./reverse1 reverse.c tmp >> pomiar_zad_2.txt
	./reverse1024 reverse.c tmp >> pomiar_zad_2.txt
	rm -f tmp
