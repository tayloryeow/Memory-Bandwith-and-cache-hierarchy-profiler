/* ------------
 * This code is provided solely for the personal and private use of 
 * students taking the CSC367H1 course at the University of Toronto.
 * Copying for purposes other than this use is expressly prohibited. 
 * All forms of distribution of this code, whether as given or with 
 * any changes, are expressly prohibited. 
 * 
 * Authors: Bogdan Simion, Felipe de Azevedo Piovezan
 * 
 * All of the files in this directory and all subdirectories are:
 * Copyright (c) 2017 Bogdan Simion
 * -------------
*/

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

float *input;
float *weights;
float *result;
int32_t n;
int32_t threads;

//Implement the next 4 functions.

// Sequential implementation of the array "input" with 
// the array "weights", writing the result to "results".
void scale_sequential()
{
    for(int i = 0; i < n; i++){
        result[i] = input[i] * weights[i];
    }
}   

// Parallel sharded implementation of the array "input" with 
// the array "weights", writing the result to "results".
void* scale_parallel_sharded(void *val)
{
    /* Remember that a thread needs an id. 
     * You should treat val as a pointer
     * to an integer representing the id of the thread.
     */
     
    int offset = (*(int *)val) * (n/threads); 

    //Loop through a n/num thread portion of the matrixes
    for (int i = offset; i < n/threads; i++){

        result[i] = input[i] * weights[i];
    }
    return NULL;
}

// Parallel strided implementation of the array "input" with 
// the array "weights", writing the result to "results".
void* scale_parallel_strided(void *val)
{
    /* Remember that a thread needs an id. 
     * You should treat val as a pointer
     * to an integer representing the id of the thread.
     */
    int id = (*(int *)val); 

    for (int i = id; i < n; i += id){
        result[i] = input[i] * weights[i];
    }
    return NULL;
}

enum{
    SHARDED,
    STRIDED
};

void start_parallel(int mode)
{
    /* Create your threads here
     * with the correct function as argument (based on mode).
     * Notice that each thread will need an ID as argument,
     * so that the thread can tell which indices of the array it should
     * work on. For example, to create ONE thread on sharded mode,
     * you would use:
     *   int id = 0;
     *   pthread_t worker;
     *   pthread_create(&worker, NULL, scale_parallel_sharded, &id);
     *
     * You want to create "thread" threads, so you probably need
     * an array of ids and an array of pthread_t.
     * Don't forget to wait for all the threads to finish before
     * returning from this function (hint: look at pthread_join()).
     */



    int ids[threads];
    pthread_t workers[threads];

    for (int i = 0; i < threads; i++){
        ids[i] = i;
        pthread_create(&workers[i], NULL, scale_parallel_sharded, &ids[i]);
    }

    for (int i = 0; i < threads; i++){
        ids[i] = pthread_join(&workers[i], NULL);
    }
}

//Don't touch this function
int read_input_from_file(char *filename, float **target)
{
    FILE *f = fopen(filename, "r");
    if (f == NULL)
    {
        printf("file doesn't exist\n");
        return -1;
    }

    int current_buffer = 1;
    int read = 0;
    *target = malloc(current_buffer * sizeof(float));
    float x;

    while (fscanf(f, "%f", &x) == 1)
    {
        if (read == current_buffer)
        {
            float *temp = malloc(2 * current_buffer * sizeof(float));
            memcpy(temp, *target, current_buffer * sizeof(float));
            current_buffer *= 2;
            free(*target);
            *target = temp;
        }

        (*target)[read] = x;
        read++;
    }

    fclose(f);
    return read;
}

//Don't touch this function
int dump_output_to_file(float *output, int size, char *filename)
{
    FILE *f = fopen(filename, "w");
    if (f == NULL)
    {
        printf("could not open output file\n");
        return -1;
    }

    int i;
    for(i = 0; i < size; i++)
    {
        fprintf(f, "%f\n", output[i]);
    }

    fclose(f);
    return 0;
}

int main(int argc, char **argv)
{
    /**************** Don't change this code **********************/
    if (argc < 4)
    {
        printf("Usage: %s inputs_file weights_file num_threads\n", argv[0]);
        exit(1);
    }

    char *inputs_filename = argv[1];
    char *weights_filename = argv[2];
    threads = atoi(argv[3]);
    n = read_input_from_file(inputs_filename, &input);

    if (n < 0 || n != read_input_from_file(weights_filename, &weights))
    {
        printf("Either filesizes don't match or error while reading from"
               "file\n");
    }

    result = malloc(n * sizeof(float));

    /**************** Change the code below **********************/

    {
        //call your sequential function here
        //and time it. Do not include the line
        //below in the measurement.
        dump_output_to_file(result, n, "sequential_output.txt");
    }
    {
        //call your start_parallel function here on strided mode
        //and time it. Do not include the line
        //below in the measurement.
        dump_output_to_file(result, n, "strided_output.txt");
    }
    {
        //call your start_parallel function here on sharded mode
        //and time it. Do not include the line
        //below in the measurement.
        dump_output_to_file(result, n, "sharded_output.txt");
    }
}
