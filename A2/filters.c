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
#include <stdlib.h>

#include <assert.h>

#include "filters.h"
#include "../../../../../usr/include/stdint.h"
#include <pthread.h>
#include <stdlib.h>


/************** FILTER CONSTANTS*****************/
/* laplacian */
int8_t lp3_m[] =
    {
        0, 1, 0,
        1, -4, 1,
        0, 1, 0,
    };
filter lp3_f = {3, lp3_m};

int8_t lp5_m[] =
    {
        -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1,
        -1, -1, 24, -1, -1,
        -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1,
    };
filter lp5_f = {5, lp5_m};

/* Laplacian of gaussian */
int8_t log_m[] =
    {
        0, 1, 1, 2, 2, 2, 1, 1, 0,
        1, 2, 4, 5, 5, 5, 4, 2, 1,
        1, 4, 5, 3, 0, 3, 5, 4, 1,
        2, 5, 3, -12, -24, -12, 3, 5, 2,
        2, 5, 0, -24, -40, -24, 0, 5, 2,
        2, 5, 3, -12, -24, -12, 3, 5, 2,
        1, 4, 5, 3, 0, 3, 5, 4, 1,
        1, 2, 4, 5, 5, 5, 4, 2, 1,
        0, 1, 1, 2, 2, 2, 1, 1, 0,
    };
filter log_f = {9, log_m};

/* Identity */
int8_t identity_m[] = {1};
filter identity_f = {1, identity_m};

filter *builtin_filters[NUM_FILTERS] = {&lp3_f, &lp5_f, &log_f, &identity_f};


/* Normalizes a pixel given the smallest and largest integer values
 * in the image */
void normalize_pixel(int32_t *target, int32_t pixel_idx, int32_t smallest, 
        int32_t largest)
{
    if (smallest == largest)
    {
        return;
    }
    
    target[pixel_idx] = ((target[pixel_idx] - smallest) * 255) / (largest - smallest);
}

/*************** COMMON WORK ***********************/
/* Process a portion of the output graph
 * Input:
 *  -Image Filter: filter matrix
 *  -original    : unpartitioned input image matrix
 *  -target      : partitioned output image matrix
 *  -width       : Widths of the original image matrix
 *  -height      : Height of the original image matrix
 *  -row         : row index of target pixel
 *  -col         : column index of target pixel
************/

int32_t apply2d(const filter *f, const int32_t *original, int32_t *target,
        int32_t width, int32_t height,
        int row, int column)
{
    int sum = 0;
    int offset = (f->dimension - 1) / 2;
    
	//Convert from 2d to 1d index's and check bounds. Aggregate and return dot product of matrix.
    for (int i = 0; i < f->dimension; i++){
        for (int j = 0; j < f->dimension; j++){
            int curr_x = column - offset + i;
            int curr_y = row - offset + j;
			//If 2d indicies are within 2d bounds
            if ((0 <= curr_x && curr_x < width) && (0 <= curr_y && curr_y < height)){
				//Write to pic array with 1d cordinates. 	
                sum += original[curr_x + curr_y * width] * f->matrix[j + i * f->dimension];
            }
        }
    }   
	target[row * width + column] = sum;
    return sum;
}

/*********SEQUENTIAL IMPLEMENTATIONS ***************/
/* for (Every pixel):
 *     filter (dot_product) original
 *
 * Two Dimensional index cords to One Dimensional index
          1d = 2d.row * width + col
 *        ________
 *       |        |          ___________________________________
 * Row ->|  x     |    ==   |________|__x_____|________|________|
 *  1    |        |    ==               ^row * width + col = 10
 *       |________|
 *          ^
 *          |Col = 2
 */
void apply_filter2d(const filter *f, 
        const int32_t *original, int32_t *target,
        int32_t width, int32_t height)
{
    int max=-9999;
    int min=9999;
    
    //Apply filter to every pixel in image.	
    for(int y = 0; y < height; y++){
        for (int x = 0; x < width; x++){

            //Apply the filter to this target element
            int sum = 0;
            int offset = (f->dimension - 1) / 2;
            for (int i = 0; i < f->dimension; i++){
                for (int j = 0; j < f->dimension; j++){
                    //Convert from 2d to 1d indicies and check bounds.
                    //Aggregate and return dot product of matrix.
                    int curr_x = x - offset + i;
                    int curr_y = y - offset + j;
                    //If 2d indicies are within 2d bounds
                    if ((0 <= curr_x && curr_x < width) && (0 <= curr_y && curr_y < height)){
                        //Write to pic array with 1d cordinates.
                        sum += original[curr_x + curr_y * width] * f->matrix[j + i * f->dimension];
                    }
                }
            }
            target[y * width + x] = sum;

            if (sum > max){max = sum;}
            if (sum < min){min = sum;}
        }
	}

    assert(min != max);
	//Normalize bitmap
    for(int tar_byte = 0; tar_byte < width * height; tar_byte++){
        normalize_pixel(target, tar_byte, min, max);
    }
    return;
}

/*Global Array of min's and max's used for normalization*/
int *min_arr;
int *max_arr;
work_queue *work_q;
int g_chunk;

/****************** ROW/COLUMN SHARDING ************/
/*
 *
 * Example for 2 threads.
 * Simple Matrix
 *     0  1  2  3
 * 0   0  1  2  3
 * 1   4  5  6  7
 * 2   8  9 10 11
 * 3  12 13 14 15
 *
 * Iteration: 0  1   2   3   4   5   6   7
 * _________|______________________________
 * Row Sharded
 *        T0: 0  1   2   3   4   5   6   7
 *        T1: 8  9  10  11  12  13  14  15
 * Column Sharded
 * ----Column Major
 *        T0: 0  4   8  12   1   5   9  13
 *        T1: 2  6  10  14   3   7  11  15
 * ----Row Major
 *        T0: 0  1   4   5   8   9  12  13
 *        T1: 2  3   6   7  10  11  14  15
 *
 * xy - cords
 *Iteration: 0  1   2   3   4   5   6   7
 * Row Sharded
 *        T0: 00  01   02   03   10   11   12   13
 *        T1: 20  21   22   23   30   31   32   33
 * Column Sharded
 * ----Column Major
 *        T0: 00  10   20   30   01   11   21   31
 *        T1: 02  12   22   32   03   13   23   33
 * ----Row Major
 *        T0: 00  01   10   11   20   21   30   31
 *        T1: 02  03   12   13   22   23   32   33
 *
 *   NOTICE: Row Sharded and columned sharede colum major is a simple coordinate reflection.
 *
 *   Row Major col-shard and Row Sharded have the same ordering of dimensions. Ie outer dimension y, inner x.
 *   Col-shar col Major is Outer dimension x inner dimension y.
 *
  */
void* sharding_work(void *pnt)
{
    /* Your algorithm is essentially:
     *  1- Apply the filter on the image
     *  2- Wait for all threads to do the same
     *  3- Calculate global smallest/largest elements on the resulting image
     *  4- Scale back the pixels of the image. For the non work queue
     *      implementations, each thread should scale the same pixels
     *      that it worked on step 1.*/

    assert(pnt != NULL);
    work * w = (work *) pnt;

    enum loop_state {FILTER, NORMALIZE};

    const filter *f = w->common->f;
    const int32_t *original = w->common->original_image;
    int32_t *output_image = w->common->output_image;
    int32_t width = w->common->width;
    int32_t height = w->common->height;
    int32_t max_threads = w->common->max_threads;

    int num_outer_dim;
    int num_inner_dim;
    int outer_idx_start;
    int outer_idx_end;
    int inner_idx_start;
    int inner_idx_end;

    int part_start;
    int part_bounds;

    int job_idx = -1;
    int state = FILTER;

    int glob_min = 9999999;
    int glob_max = -9999999;

    int exit_flag = 0;

    do {
        //Cut the partition. Each end-start pair represents bound of partition allocation along an axis
        switch ((int) *w->method){
            case SHARDED_ROWS:
                /*Outer dimension: x-axis. Inner Dimension: y-axis.*/
                num_outer_dim = (int) ((height + max_threads - 1) / max_threads);
                num_inner_dim = width;
                outer_idx_start = num_outer_dim * w->id;
                inner_idx_start = 0;

                part_bounds = height;
                part_start = outer_idx_start;
                break;
            case SHARDED_COLUMNS_COLUMN_MAJOR:
                /*Outer dimension: x-axis. Inner Dimension: y-axis.*/
                num_outer_dim = (int)((width + max_threads - 1)/max_threads);
                num_inner_dim = height;
                outer_idx_start = num_outer_dim * w->id;
                inner_idx_start = 0;

                part_bounds = width;
                part_start = outer_idx_start;
                break;

            case SHARDED_COLUMNS_ROW_MAJOR:
                num_outer_dim = height;
                num_inner_dim = (int) ((width + max_threads - 1) / max_threads);
                outer_idx_start = 0;
                inner_idx_start = w->id * num_inner_dim;

                part_bounds = width;
                part_start = inner_idx_start;
                break;
            case WORK_QUEUE:
                num_outer_dim = g_chunk;
                num_inner_dim = g_chunk;
                //Get 1d job idx and translate that to 2d cordinate
                job_idx = dequeue_job(work_q);
                inner_idx_start = job_idx % width;
                outer_idx_start = job_idx / width;

                //Shortcircuit filter loop is workqueue is empty
                if (job_idx < 0){
                    part_start = 0;
                    part_bounds = part_start - 1;
                }else{
                    part_start = 0;
                    part_bounds = part_start + 1;
                }
                break;
            default:
                exit(1);
        }


        //Disqualify bad partitions - Shortcircuit work for-loops without affecting barrier/syncro operations
        if (part_start >= part_bounds){
            outer_idx_end = outer_idx_start;
            inner_idx_end = inner_idx_start;
        }
        else{
            outer_idx_end = outer_idx_start + num_outer_dim;
            inner_idx_end = inner_idx_start + num_inner_dim;
        }

        int min = 999999999;
        int max = -999999999;

        //Loop over every point of the output array
        //State = 0. Filter it
        //state = 1. Normalize it
        for ( ; state < 2; state++) {
            //Loop over the outside dimension
            for (int outer_dim = outer_idx_start; outer_dim < outer_idx_end; outer_dim++) {
                for (int inner_dim = inner_idx_start; inner_dim < inner_idx_end; inner_dim++) {
                    int pixel;
                    int tar_col;
                    int tar_row;

                    //Translate loop indicies to cordinates coresponding to original imge
                    switch ((int) *w->method) {
                        case WORK_QUEUE:

                        case SHARDED_ROWS:
                        case SHARDED_COLUMNS_ROW_MAJOR:
                            tar_row = outer_dim;
                            tar_col = inner_dim;
                            break;
                        case SHARDED_COLUMNS_COLUMN_MAJOR:
                            /*Reflects the direction of iteration to iterate over columns instead of row*/
                            tar_row = inner_dim;
                            tar_col = outer_dim;
                            break;
                        default:
                            exit(1);
                    }


                    //Discard bad target elements: Only possible on last iteration
                    if (tar_row * width + tar_col >= width * height) { break; }

                    switch (state) {
                        case FILTER:
                            pixel = apply2d(f, original, output_image, width, height, tar_row, tar_col);
                            (pixel > max) ? max = pixel : max;
                            (pixel < min) ? min = pixel : min;
                            break;
                        case NORMALIZE:
                            normalize_pixel(output_image, tar_row * width + tar_col, glob_min, glob_max);
                            break;
                    }
                }
            }/*End Over every pixel loop*/

            //Skip Normalization step on method Work Queue.
            // WQ only processes partitions until all are processed
            //Normalization is similarily partitioned
            if (*w->method != WORK_QUEUE){
                //Update extrema data needed for by other threads and wait until everyone's finished
                min_arr[w->id] = min;
                max_arr[w->id] = max;
                glob_min = 999999;
                glob_max = -999999;
                pthread_barrier_wait(&w->common->barrier);
                //Obtain the extrema across all the threads
                for (int thread = 0; thread < max_threads; thread++) {
                    (glob_min > min_arr[thread]) ? glob_min = min_arr[thread] : glob_min;
                    (glob_max < max_arr[thread]) ? glob_max = max_arr[thread] : glob_max;
                }

            }
            //Work Queue can't normalize immediately so short circuit loop
            //by shifting for loop variable state: immeditately reset state
            //outside loop so next iteration of loop behaves sensibly
            else {
                break;
            }
        }/*End Filter/Normalization loop*/

        if (*w->method == WORK_QUEUE) {
            //Filtering and jobs left
            if (state == FILTER && job_idx >= 0) {
                if (min_arr[w->id] > min) { min_arr[w->id] = min; }
                if (max_arr[w->id] < max) { max_arr[w->id] = max; }
                state = FILTER;
            }
            //Finished Filtering - wait on other threads and get extrema
            else if (state == FILTER && job_idx < 0){
                //Get Local and
                pthread_barrier_wait(&w->common->barrier);
                work_q->index = 0;
                pthread_barrier_wait(&w->common->barrier);

                //Set Global Min and Max nothing changes these values after this line.
                glob_min = 999999;
                glob_max = -999999;
                //Obtain the extrema across all the threads
                for (int thread = 0; thread < max_threads; thread++) {
                    (glob_min > min_arr[thread]) ? glob_min = min_arr[thread] : glob_min;
                    (glob_max < max_arr[thread]) ? glob_max = max_arr[thread] : glob_max;
                }
                state = NORMALIZE;
            }
            //Normal Normalizing processing
            else if (state == NORMALIZE && job_idx >= 0) {
                state = NORMALIZE;
            }
            //Finished Normalizing Finished Filtering.
            else if (state == NORMALIZE && job_idx < 0){
                state = 3;
                exit_flag = 1;
            }
            else {
                perror ("Unexpected failure");
                return NULL;
            }
        }
    //Continue loop until Thread Pool has both Filtered the image and Normalized it
    } while (*w->method == WORK_QUEUE && !exit_flag);
    return NULL;
}

/**
 *
 * @param q struct describing a shared int representing dequeued job number
 * @return 1d coordinate of top left index of this job partition. -1 if empty
 */
int dequeue_job(work_queue *q)
{
    int temp;
    pthread_mutex_lock(&q->lock);
    temp = q->jobs[q->index];
    if (q->index > q->last_idx){
        temp = -1;
    }
    else {
        q->index++;
    }
    pthread_mutex_unlock(&q->lock);

    return temp;
}

/***************** WORK QUEUE *******************/
/* TODO: you don't have to implement this. It is just a suggestion for the
 * organization of the code.
 * You will now implement a Work Pool implementation using pthreads.
 * This is specified as the WORK_QUEUE method number. In the WORK_QUEUE method,
 * the image is divided in square tiles of size chunk x chunk (where chunk is the argument
 * described as the work chunk in part 1). All the tiles (work chunks) are statically placed
 * in a queue at the start of the program. A task has the granularity of one tile. That is, each
 * thread will take one tile at a time from the queue and process it before proceeding to
 * grab another tile. You must implement this abstraction efficiently.

    Note: Your implementation must ensure that accesses to shared resources are synchronized.
    Keep in mind that although your program may be run in sequential mode (that is, using a work pool with
    1 thread), you should still use locking where necessary.

    This part consists of implementing the remaining part of the function apply_filter2d_threaded in filters.c,
    in other words, you should handle the case where method is WORK_QUEUE.
 */

//Fills work_q 1d index of the top left index of sqsuare partitions
void queue_work(void *pntr)  {
    common_work *w = (common_work *) pntr;

    int last_idx = 0;
    for (int j_y = 0; j_y < (w->width+g_chunk-1)/g_chunk ; j_y++) {
        for (int j_x = 0; j_x < w->height/g_chunk; j_x++) {
            double value = (j_y * g_chunk * w->width) + j_x * g_chunk;
            int ind = j_y * w->width/g_chunk + j_x;
            work_q->jobs[ind] = value;
            last_idx = ind;
        }
    }
    work_q->last_idx = last_idx;
}


/***************** MULTITHREADED ENTRY POINT ******/
/* TODO: this is where you should implement the multithreaded version
 * of the code. Use this function to identify which method is being used
 * and then call some other function that implements it.
 */
void apply_filter2d_threaded(const filter *f,
        const int32_t *original, int32_t *target,
        int32_t width, int32_t height,
        int32_t num_threads, parallel_method method, int32_t work_chunk)
{
    //Instantiate Barrier
    pthread_barrier_t barrier;
    unsigned count = num_threads;
    int retval = pthread_barrier_init(&barrier, NULL, count);
    if (retval){
        perror("Memory Barrier Creation Failure");
        exit(1);
    }

    //Create thread infromation arrays
    pthread_t threads[num_threads];
    work thread_info[num_threads];
    common_work work_info = {f, original, target, width, height, num_threads, barrier};

    //Initialize Global Variables
    min_arr = malloc(sizeof(int) * num_threads);
    max_arr = malloc(sizeof(int) * num_threads);


    if (method == WORK_QUEUE) {
        g_chunk = work_chunk;
        work_q = malloc(sizeof(work_queue));
        work_q->jobs = malloc(sizeof(int) * width/g_chunk * width/g_chunk);
        work_q->index = 0;
        pthread_mutex_init(&(work_q->lock), NULL);
        queue_work(&work_info);
    }

    //Create N Threads
    for (int t = 0; t < num_threads; t++){
        work w = {&work_info, t, &method};
        thread_info[t] = w;
        retval = pthread_create(&threads[t], NULL, sharding_work, &thread_info[t]);
        if (retval){
            perror("PthreadCreate");
            pthread_exit(NULL);
        }
    }

    //Join N threads
    for(int t = 0; t < num_threads; t++) {
        pthread_join(threads[t], NULL);
    }
    free(min_arr);
    free(max_arr);
    if (method == WORK_QUEUE) {
        free(work_q->jobs);
        free(work_q);
    }

    //Sanity Check ouput
    for (int i = 0; i < height * width; i++){
        if (target[i] < 0 || target[i] > 255){
            printf("unnormalized pmg outputted\n");
            exit(1);
        }
    }
    pthread_barrier_destroy(&barrier);
    return;
}
