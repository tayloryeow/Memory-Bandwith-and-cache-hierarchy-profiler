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
 *
 * Two Dimensional index cords to One Dimensional index
          1d = 2d.row * width + col
 *        ________
 *       |        |          ___________________________________
 * Row ->|  x     |    ==   |________|__x_____|________|________|
 *  1    |        |    ==               ^row * width + col = 10
 *       |________|
 *          ^
 *          |
 *          C
 *          o 2
 *          l
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

/****************** ROW/COLUMN SHARDING ************/
/* TODO: you don't have to implement this. It is just a suggestion for the
 * organization of the code.
 */

/* Recall that, once the filter is applied, all threads need to wait for
 * each other to finish before computing the smallest/largets elements
 * in the resulting matrix. To accomplish that, we declare a barrier variable:
 *      pthread_barrier_t barrier;
 * And then initialize it specifying the number of threads that need to call
 * wait() on it:
 *      pthread_barrier_init(&barrier, NULL, num_threads);
 * Once a thread has finished applying the filter, it waits for the other
 * threads by calling:
 *      pthread_barrier_wait(&barrier);
 * This function only returns after *num_threads* threads have called it.
 */
void* sharding_work(void *pnt)
{
    /* Your algorithm is essentially:
     *  1- Apply the filter on the image
     *  2- Wait for all threads to do the same
     *  3- Calculate global smallest/largest elements on the resulting image
     *  4- Scale back the pixels of the image. For the non work queue
     *      implementations, each thread should scale the same pixels
     *      that it worked on step 1.
       ______
    * |______|  T0 -> 0 - width : height = 1
    * |______|  T1 -> width - 2width: height = 1
    * |______|  T2 -> 2width - 3width: height = 1
    * |______|  T3 -> 3width - 4width:
    *  While the partitions should technically be from 0 - width - 1. As the width element should be part
    *  of the second shard. However this would force T0's right most edges to be out of
    *
    * */
    assert(pnt != NULL);
    work * w = (work *) pnt;

    const filter *f = w->common->f;
    const int32_t *original = w->common->original_image;
    int32_t *output_image = w->common->output_image;
    int32_t width = w->common->width;
    int32_t height = w->common->height;
    int32_t max_threads = w->common->max_threads;
    pthread_barrier_t barrier = w->common->barrier;

    int row_per_part = ((height + max_threads - 1) / max_threads);

    int start_row = row_per_part * w->id;
    int min = 999999999;
    int max = -999999999;

    //Loop over the output partition determined by Thread_id
    for (int row = start_row; row < start_row + row_per_part; row++) {
        for (int col = 0; col < width; col++) {
            //Discontinue if target is out of bounds of image.
            //Possible on last iteration


            int pixel;
            pixel = apply2d(f, original, output_image, width, height, row, col);
            if (pixel > max){max = pixel;}
            if (pixel < min){min = pixel;}
        }
    }

    pthread_barrier_wait(&barrier);

    //Loop over the output partition determined by Thread_id
    for (int row = start_row; row < start_row + row_per_part; row++) {
        for (int col = 0; col < width; col++) {
            //Discontinue if target is out of bounds of image.
            //Possible on last iteration
            normalize_pixel(output_image, row*width + col, min, max);
        }
    }

    return NULL;
}

/***************** WORK QUEUE *******************/
/* TODO: you don't have to implement this. It is just a suggestion for the
 * organization of the code.
 */
void* queue_work(void *work)
{
    return NULL;
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

    //Create N Threads
    pthread_t threads[num_threads];
    work thread_info[num_threads];
    common_work work_info = {f, original, target, width, height, num_threads, barrier};
    for (int t = 0; t < num_threads; t++){
        work w = {&work_info, t};
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

    //Sanity Check ouput
    for (int i = 0; i < height * width; i++){
        if (target[i] < 0 || target[i] > 255){
            printf("unnormalized pmg outputted\n");
            exit(1);
        }
    }
    return;
}
