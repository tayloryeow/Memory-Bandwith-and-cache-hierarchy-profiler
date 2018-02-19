SHELL := /bin/bash 

CC=gcc
GCC_OPT-N=-Wall -Werror -g
GCC_OPT=-pthread -O2 -Wall -Werror

WIDTH=4294967296
HEIGHT=4294967296

PMG_DIR="pgm/"


pgm_creator:
	$(CC) $(GCC_OPT) pgm_creator.c pgm.c -o pgm_creator.out


%.o: %.c
	$(CC) -c -o $@ $< $(GCC_OPT)

main: very_big_sample.o very_tall_sample.o main.c pgm.c filters.c
	$(CC) $(GCC_OPT) main.c pgm.c filters.c very_big_sample.o very_tall_sample.o -o main.out -lpthread

all: pgm_creator main sequential_test parallel_test

debug: main pgm_creator 
	gdb --tui --args main.out -f1 -b2 -m1 -o out.pgm

parallel_test: pgm_creator main
	./pgm_creator.out $(WIDTH) $(HEIGHT) 
	./main.out -b1 -n10 -f1 -m2 -o outb1f1m2.pgm
	./main.out -b2 -n10 -f1 -m2 -o outb2f1m2.pgm
	./main.out -b1 -n10 -f2 -m2 -o outb1f2m2.pgm
	./main.out -b2 -n10 -f2 -m2 -o outb2f2m2.pgm
	./main.out -b1 -n10 -f3 -m2 -o outb1f3m2.pgm
	./main.out -b2 -n10 -f3 -m2 -o outb2f3m2.pgm
	./main.out -b1 -n10 -f4 -m2 -o outb1f4m2.pgm
	./main.out -b2 -n10 -f4 -m2 -o outb2f4m2.pgm
	./test_pgms.sh $(PMG_DIR) 2
	./main.out -b1 -n10 -f1 -m3 -o outb1f1m3.pgm
	./main.out -b2 -n10 -f1 -m3 -o outb2f1m3.pgm
	./main.out -b1 -n10 -f2 -m3 -o outb1f2m3.pgm
	./main.out -b2 -n10 -f2 -m3 -o outb2f2m3.pgm
	./main.out -b1 -n10 -f3 -m3 -o outb1f3m3.pgm
	./main.out -b2 -n10 -f3 -m3 -o outb2f3m3.pgm
	./main.out -b1 -n10 -f4 -m3 -o outb1f4m3.pgm
	./main.out -b2 -n10 -f4 -m3 -o outb2f4m3.pgm	
	./test_pgms.sh $(PMG_DIR) 3
	./main.out -b1 -n10 -f1 -m4 -o outb1f1m4.pgm
	./main.out -b2 -n10 -f1 -m4 -o outb2f1m4.pgm
	./main.out -b1 -n10 -f2 -m4 -o outb1f2m4.pgm
	./main.out -b2 -n10 -f2 -m4 -o outb2f2m4.pgm
	./main.out -b1 -n10 -f3 -m4 -o outb1f3m4.pgm
	./main.out -b2 -n10 -f3 -m4 -o outb2f3m4.pgm
	./main.out -b1 -n10 -f4 -m4 -o outb1f4m4.pgm
	./main.out -b2 -n10 -f4 -m4 -o outb2f4m4.pgm	

	./test_pgms.sh $(PMG_DIR) 4

sequential_test: main pgm_creator
	./pgm_creator.out $(WIDTH) $(HEIGHT)
	./main.out -b1 -f1 -m1 -o outb1f1m1.pgm
	./main.out -b2 -f1 -m1 -o outb2f1m1.pgm
	./test_pgms.sh $(PMG_DIR) 1

c4eanpgm:
	rm *.pgm	

clean:
	rm *.o *.out *.pgm
