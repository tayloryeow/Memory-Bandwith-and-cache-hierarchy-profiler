CFLAGS = "-o -ftrapv -Wall"
HEADERS = ''
CC = "gcc"
TESTNAME = "test2"

all: part1.o
	$(CC) -o tester part1.o

test:  
	echo "running test"
	./tester "TESTNAME 10 data/test1"
graph: part1.o 
	python2 graph.py $(TESTNAME)

%.o: %.c $(HEADERS)
	$(CC) -c $@ $< $(CFLAGS)

clean:
	rm *.o
