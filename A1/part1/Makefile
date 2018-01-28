CFLAGS = "-o -ftrapv -Wall"
HEADERS = ''
CC = "gcc"

all: part1.o
	$(CC) -o tester part1.o

test:  
	echo "running test"
	./tester "test1 10 data/test1"

%.o: %.c $(HEADERS)
	$(CC) -c $@ $< $(CFLAGS)

clean:
	rm *.o
