CC=gcc
CFLAGS=-Wall

all: replace_std replace_sys

replace_std:
	$(CC) $(CFLAGS) $(TESTING) -o replace_std replace_std.c

replace_sys:
	$(CC) $(CFLAGS) $(TESTING) -o replace_sys replace_sys.c

clean:
	rm *a *o replace_std replace_sys

test: replace_std replace_sys
	rm -f pomiar_zad_1.txt
	make all TESTING="-D TESTING"
	./replace_std a o replace_std.c tmp >> pomiar_zad_1.txt
	./replace_std a o replace_sys.c tmp >> pomiar_zad_1.txt
	./replace_sys a o replace_std.c tmp >> pomiar_zad_1.txt
	./replace_sys a o replace_sys.c tmp >> pomiar_zad_1.txt
	rm -f tmp
