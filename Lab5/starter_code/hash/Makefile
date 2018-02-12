# ------------
# This code is provided solely for the personal and private use of
# students taking the CSC367H1 course at the University of Toronto.
# Copying for purposes other than this use is expressly prohibited.
# All forms of distribution of this code, whether as given or with
# any changes, are expressly prohibited.
#
# Authors: Bogdan Simion, Alexey Khrabrov
#
# All of the files in this directory and all subdirectories are:
# Copyright (c) 2017 Bogdan Simion
# -------------

CC = gcc
CFLAGS += -std=gnu11 -Wall -Werror -g3 -O3 -DNDEBUG
LDFLAGS += -pthread -lm

all: test-simple test-perf test-perf-parallel-mutex test-perf-parallel-rwlock

hash-mutex.o: hash.h
hash-rwlock.o: hash.h
test-perf.o: time_util.h
test-perf-parallel.o: time_util.h

test-simple: test-simple.o hash-rwlock.o
	$(CC) $^ -o $@ $(LDFLAGS)

test-perf: test-perf.o hash-rwlock.o
	$(CC) $^ -o $@ $(LDFLAGS)

test-perf-parallel-mutex: test-perf-parallel.o hash-mutex.o
	$(CC) $^ -o $@ $(LDFLAGS)

test-perf-parallel-rwlock: test-perf-parallel.o hash-rwlock.o
	$(CC) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o test-simple test-perf test-perf-parallel-mutex test-perf-parallel-rwlock
