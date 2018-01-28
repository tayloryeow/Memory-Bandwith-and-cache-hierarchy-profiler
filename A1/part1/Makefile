CFLAGS = "-o -ftrapv -Wall"
HEADERS = ''
CC = "gcc"

TEST="default"
NUM_SAMPLES=10
F_PATH="data/"$(TEST)

all: part1.o
	$(CC) -o tester part1.o

test:  
	echo "running test"
	./tester $(TEST) $(NUM_SAMPLES) $(F_PATH) 
graph:
	echo "Graphing..."
	python2 graph.py $(TEST) 
	xdg-open examples/charts/$(TEST).svg



%.o: %.c $(HEADERS)
	$(CC) -c $@ $< $(CFLAGS)

clean:
	rm *.o
