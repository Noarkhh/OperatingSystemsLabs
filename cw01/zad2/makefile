CC=gcc
CFLAGS=-Wall

all: main_static main_shared main_dynamic

lib_shared:
	$(CC) $(CFLAGS) -fPIC -c wc_repl_lib.c
	$(CC) $(CFLAGS) -fPIC -c wc_block_lib.c
	$(CC) $(CFLAGS) -fPIC -shared -o lib_wc_repl_lib.so wc_repl_lib.o wc_block_lib.o

lib_static:
	$(CC) $(CFLAGS) -c wc_repl_lib.c
	$(CC) $(CFLAGS) -c wc_block_lib.c
	ar rcs lib_wc_repl_lib.a wc_repl_lib.o wc_block_lib.o

main_static: lib_static
	$(CC) $(CFLAGS) -o main_static main.c lib_wc_repl_lib.a

main_shared: lib_shared
	$(CC) $(CFLAGS) -o main_shared main.c -L. -l_wc_repl_lib

main_dynamic: lib_shared
	$(CC) $(CFLAGS) -o main_dynamic main_dynamic.c

clean:
	rm *o *a main_static main_shared main_dynamic
