CC=gcc
CFLAGS=-Wall

all: main_shared main_static

lib_shared:
	$(CC) $(CFLAGS) -fPIC -c wc_block_lib.c
	$(CC) $(CFLAGS) -fPIC -shared -o lib_wc_block_lib.so wc_block_lib.o

lib_static:
	$(CC) $(CFLAGS) -c wc_block_lib.c
	ar rcs lib_wc_block_lib.a wc_block_lib.o

main_shared: lib_shared
	$(CC) $(CFLAGS) -o main_shared main.c -L. -l_wc_block_lib

main_static: lib_static
	$(CC) $(CFLAGS) -o main_static main.c lib_wc_block_lib.a

clean:
	rm *o *a main_shared main_static