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

#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int num_barbers; //command line argument
int total_customers; //command line argument
int waiting_room_capacity; // command line argument
int *waiting_room;

//first is the index of the first customer in "line", last is the index of the
//customer who arrived last
int first, last;  

//1 customer roughly every ARRIVAL_AVERAGE units;
int arrival_average = ARRIVAL_AVERAGE;

int in_waiting_room;
int customers_served;
int customers_angry;
pthread_mutex_t mutex;

/* Warning: do NOT call this while holding any locks;
 *          do NOT modify this function.
 */
void cut(int barber, int customer)
{
    /* We want the speed in which customers arrive to be
    * slightly faster than the speed in which the whole barbershop can
    * serve them, for example:
    *   arrival_speed = 1.3 * serving_speed
    *   1 / arrival_average  = 1.3 * num_barbers / cut_time
    * So the time to serve one costumer should be:
    *   cut_time = 1.3 * num_barbers * arrival_average;
    * We multiply it by 2 so that we get the desired result 
    * on average.
    */
    printf("Barber %d serving customer %d\n", barber, customer);
    usleep(rand() % (int) (1.3 * num_barbers * arrival_average * 2)); 
}

void* barber_mainloop(void *args)
{
    int id = *((int*)args);
    /* TODO: Implement this function
     * A barber will:
     *  1- Check the waiting room. If a customers are there, the one waiting
     *  for the longest time is served.
     *  2- If the waiting room is empty and not ALL customers have arrived,
     *      the barber checks again.
     *  3- If the waiting room is empty and ALL customers have arrived for the day,
     *      the barber goes home (exits).
     */
}

/* DO NOT MODIFY ANYTHING BELOW THIS POINT */
void customers_mainloop()
{
    int customers_created = 0;

    while(customers_created < total_customers)
    {
        customers_created++;

        usleep(rand() % (2 * arrival_average));
        pthread_mutex_lock(&mutex);
        if (in_waiting_room == waiting_room_capacity)
        {
            printf("Customer %d went home angry\n", customers_created);
            customers_angry++;
        }
        else
        {
            waiting_room[last] = customers_created;
            last = (last+1) % waiting_room_capacity;
            in_waiting_room++;
        }

        pthread_mutex_unlock(&mutex);
    }
}


int main(int argc, char **argv)
{

    if (argc < 4)
    {
        printf("Usage: %s number_of_barbers number_of_customers waiting_room_capacity"
                " as arguments\n", argv[0]);
        exit(1);
    }

    num_barbers = atoi(argv[1]);
    total_customers = atoi(argv[2]);
    waiting_room_capacity = atoi(argv[3]);
    waiting_room = malloc(waiting_room_capacity * sizeof(int));

    in_waiting_room = 0;
    customers_served = 0;
    customers_angry = 0;
    first = 0;
    last = 0;
    pthread_mutex_init(&mutex, NULL);
    srand(17);

    pthread_t barbers[num_barbers];
    int ids[num_barbers];
    int i;

    struct timespec start, stop;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (i = 0; i < num_barbers; i++)
    {
        ids[i] = i;
        pthread_create(&barbers[i], NULL, barber_mainloop, &ids[i]);
    }

    customers_mainloop();

    for (i = 0; i < num_barbers; i++)
    {
        pthread_join(barbers[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &stop);
    float elapsed = (stop.tv_sec - start.tv_sec) + 
        (stop.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("Time taken = %10.8f\n", elapsed);
}
