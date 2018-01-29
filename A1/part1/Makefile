CFLAGS += -std=gnu11 -Wall -Werror -g3 -O3 -DNDEBUG
HEADERS = ''
CC = gcc

N=10
F_PATH=data/$(TEST)
SIZE=1024
TEST=$(SIZE)Bx$(N)n

all: usage part1 
	
part1: part1.o
	$(CC) $^ -o tester

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

test: usage 
	echo "running test"
	./tester $(TEST) $(N) $(F_PATH) $(SIZE)
graph: usage 
	echo "Graphing..."
	python2 graph.py $(TEST) 
	xdg-open examples/charts/$(TEST).svg

testGraph: usage part1.o test graph

usage:
	echo "TEST:" $(TEST) "NUM_SAMPLES:" $(N) "F_PATH:" $(F_PATH) "SIZE:" $(SIZE)


clean:
	rm *.o
