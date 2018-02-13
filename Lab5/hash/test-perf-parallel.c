// ------------
// This code is provided solely for the personal and private use of
// students taking the CSC367H1 course at the University of Toronto.
// Copying for purposes other than this use is expressly prohibited.
// All forms of distribution of this code, whether as given or with
// any changes, are expressly prohibited.
//
// Authors: Bogdan Simion, Alexey Khrabrov
//
// All of the files in this directory and all subdirectories are:
// Copyright (c) 2017 Bogdan Simion
// -------------

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "hash.h"
#include "time_util.h"


static int hash_table_size;
static int keys_per_bucket;
// Each thread must execute this number of operations
static int operations_count;
static int write_percentage;
static int threads_count;

static void print_usage(char *const argv[])
{
	printf("usage: %s <hash table size> <keys per bucket> <operations count> <write percentage> <threads count>\n",
	       argv[0]);
}

static bool parse_args(int argc, char *const argv[])
{
	if (argc < 6) return false;

	hash_table_size  = atoi(argv[1]);
	keys_per_bucket  = atoi(argv[2]);
	operations_count = atoi(argv[3]);
	write_percentage = atoi(argv[4]);
	threads_count    = atoi(argv[5]);

	return (hash_table_size > 0) && (keys_per_bucket > 0) && (operations_count > 0) &&
	       (write_percentage > 0) && (write_percentage <= 100) && (threads_count > 0);
}


static bool mixed_read_write_test(hash_table_t *hash, int shift)
{
	assert(hash != NULL);
	assert(shift != 0);

	int max_key = hash_table_size * keys_per_bucket + 1;
	bool result = true;

	for (int i = 0; i < operations_count; i++) {
		bool write = (random() % 100 < write_percentage);
		int key = (int)(random() % max_key);

		if (write) {
			if (hash_put(hash, key, key + shift) != 0) {
				fprintf(stderr, "hash_put(%d, %d) failed\n", key, key + shift);
				return false;
			}
		} else {
			int value = hash_get(hash, key);
			if ((value != key + shift) && (value != -1)) {// -1 is valid since the key might not be in the table
				fprintf(stderr, "hash_get(%d) returned %d, expected %d or -1\n", key, value, key + shift);
				result = false;
			}
		}
	}
	return result;
}

static pthread_barrier_t barrier;

static bool read_after_write_test(hash_table_t *hash, int shift)
{
	assert(hash != NULL);
	assert(shift != 0);

	int max_key = hash_table_size * keys_per_bucket + 1;
	bool result = true;

	int write_count = (operations_count / 100) * write_percentage;
	int read_count = operations_count - write_count;

	for (int i = 0; i < write_count; i++) {
		int key = (int)(random() % max_key);

		if (hash_put(hash, key, key + shift) != 0) {
			fprintf(stderr, "hash_put(%d, %d) failed\n", key, key + shift);
			return false;
		}
	}

	pthread_barrier_wait(&barrier);

	for (int i = 0; i < read_count; i++) {
		int key = (int)(random() % max_key);

		int value = hash_get(hash, key);
		if ((value != key + shift) && (value != -1)) {// -1 is valid since the key might not be in the table
			fprintf(stderr, "hash_get(%d) returned %d, expected %d or -1\n", key, value, key + shift);
			result = false;
		}
	}
	return result;
}


typedef struct _thread_args_t {
	bool (*f)(hash_table_t*, int);
	hash_table_t *hash;
	int shift;
} thread_args_t;

static void *thread_f(void *arg)
{
	assert(arg != NULL);
	thread_args_t *args = arg;
	return args->f(args->hash, args->shift) ? NULL : (void*)-1;
}

static double do_perf_test(bool (*f)(hash_table_t*, int))
{
	hash_table_t *hash = hash_create(next_prime(hash_table_size));
	if (hash == NULL) exit(1);
	int shift = random() + 1;

	pthread_t threads[threads_count];
	thread_args_t args[threads_count];
	for (int i = 0; i < threads_count; i++) {
		args[i] = (thread_args_t){ f, hash, shift };
	}
	bool success = true;

	struct timespec start, end;
	clock_gettime(CLOCK_MONOTONIC, &start);

	for (int i = 0; i < threads_count; i++) {
		int rc = pthread_create(&(threads[i]), NULL, thread_f, &(args[i]));
		if (rc != 0) {
			errno = rc;
			perror("pthread_create");
			exit(1);
		}
	}
	for (int i = 0; i < threads_count; i++) {
		void *result;
		pthread_join(threads[i], &result);
		if (result != NULL) success = false;
	}

	clock_gettime(CLOCK_MONOTONIC, &end);	

	hash_destroy(hash);
	if (!success) exit(1);
	return timespec_to_msec(difftimespec(end, start));
}


int main(int argc, char *argv[])
{
	if (!parse_args(argc, argv)) {
		print_usage(argv);
		return 1;
	}

	pthread_barrier_init(&barrier, NULL, threads_count);

	srandom(time(NULL));
	printf("%f\n", do_perf_test(mixed_read_write_test));
	printf("%f\n", do_perf_test(read_after_write_test));
	return 0;
}
