CC=gcc
GCC_OPT=-Wall -Werror -g
GCC_OPT-N=-O2 -Wall -Werror 

WIDTH=4294967296
HEIGHT=4294967296

PMG_DIR="pgm/"

%.o: %.c
	$(CC) -c -o $@ $< $(GCC_OPT)

main: very_big_sample.o very_tall_sample.o main.c pgm.c filters.c
	$(CC) $(GCC_OPT) main.c pgm.c filters.c very_big_sample.o very_tall_sample.o -o main.out -lpthread

#TODO: run your script that runs the experiments, collects all necessary data, and produces the graphs
#run: all
# e.g. ./my_script.sh ... 

all: pgm_creator main sequential_test

debug: main pgm_creator 
	gdb --tui --args main.out -f1 -b2 -m1 -o out.pgm

sequential_test: main pgm_creator
	./pgm_creator.out $(WIDTH) $(HEIGHT)
	./main.out -b1 -f1 -m1 -o outb1f1.pgm
	./main.out -b2 -f1 -m1 -o outb2f1.pgm
	./test_pgms.sh $(PMG_DIR)

pgm_creator:
	$(CC) $(GCC_OPT) pgm_creator.c pgm.c -o pgm_creator.out

cleanpgm:
	rm *.pgm	


clean:
	rm *.o *.out *.pgm
