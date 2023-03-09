CC=gcc
CFLAGS=-Wall

all: test

lib_shared:
	$(CC) $(CFLAGS) $(OPTLVL) -fPIC -c wc_repl_lib.c
	$(CC) $(CFLAGS) $(OPTLVL) -fPIC -c wc_block_lib.c
	$(CC) $(CFLAGS) $(OPTLVL) -fPIC -shared -o lib_wc_repl_lib.so wc_repl_lib.o wc_block_lib.o

lib_static:
	$(CC) $(CFLAGS) $(OPTLVL) -c wc_repl_lib.c
	$(CC) $(CFLAGS) $(OPTLVL) -c wc_block_lib.c
	ar rcs lib_wc_repl_lib.a wc_repl_lib.o wc_block_lib.o

main_static: lib_static
	$(CC) $(CFLAGS) $(OPTLVL) -o main_static main.c lib_wc_repl_lib.a

main_shared: lib_shared
	$(CC) $(CFLAGS) $(OPTLVL) -o main_shared main.c -L. -l_wc_repl_lib

main_dynamic: lib_shared
	$(CC) $(CFLAGS) $(OPTLVL) -o main_dynamic main_dynamic.c

tests:
	make lib_static
	make lib_shared
	make main_static
	make main_shared
	make main_dynamic

	./main_static < test_input > result_static_$(OPTLVL).txt
	./main_shared < test_input > result_shared_$(OPTLVL).txt
	./main_dynamic < test_input > result_dynamic_$(OPTLVL).txt

test:
	make tests OPTLVL=-O
	make tests OPTLVL=-O1
	make tests OPTLVL=-O2
	make tests OPTLVL=-O3
	make tests OPTLVL=-O0
	make tests OPTLVL=-Os

clean:
	rm *o *a main_static main_shared main_dynamic *.txt
