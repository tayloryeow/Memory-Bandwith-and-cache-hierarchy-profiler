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

int total_customers;
int waiting_room_capacity;
int *waiting_room;
//first is the index of the first customer in "line", last is the index of the
//customer who arrived last
int first, last;  

int num_barbers;
//1 customer roughly every ARRIVAL_AVERAGE units;
int arrival_average = ARRIVAL_AVERAGE;

int in_waiting_room;
int customers_served;
int customers_angry;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t room_not_empty = PTHREAD_COND_INITIALIZER;

/* do NOT modify this function. */
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
    usleep(rand() % (int)(2 * num_barbers * arrival_average * 1.3));
    printf("Barber %d serving customer %d\n", barber, customer);
}

/* Check the waiting room to see if there is a customer waiting.
 * If so, cut their hair. If not AND if there are customers
 * incoming today, wait for them. Otherwise, return -1.
 * Returns customer number if a customer was served,
 * Returns -1 there are no more customers coming today. */
int check_room(int id)
{
    int customer = -1;

    /* TODO:
     * Add your code here.
     */




    return customer;
}

//Don't modify this function
void* barber_mainloop(void *args)
{
    int id = *((int*)args);
    int customer;
    //sleep so that they are not all synchronized
    usleep(rand() % (2*arrival_average));

    while((customer = check_room(id)) >= 0)
    {
        cut(id, customer);
    }
}


/* Customer arrives and calls a barber if
 * one is available. Otherwise, if there is space,
 * wait in the waiting room. Otherwise go home
 * angry
 */
void* customers_mainloop(void *args)
{
    int id = (long) args;


    /* TODO: insert your code here.
     * Somewhere in this function, print this:
     */
    printf("Customer %d went home angry\n", id);





    return NULL;
}

/* DO NOT MODIFY ANYTHING BELOW THIS POINT */
int main(int argc, char **argv)
{
    if (argc < 4)
    {
        printf("Usage: %s number_of_barbers number_of_customers "
                "waiting_room_capacity as arguments\n", argv[0]);
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
    srand(17);

    pthread_t barbers[num_barbers];
    int ids[num_barbers];
    int i;

    struct timespec start, stop;
    clock_gettime(CLOCK_REALTIME, &start);
    for (i = 0; i < num_barbers; i++)
    {
        ids[i] = i;
        pthread_create(&barbers[i], NULL, barber_mainloop, &ids[i]);
    }

    /* create costumers */
    long customers_created = 0;
    pthread_t customers[total_customers];

    while(customers_created < total_customers)
    {
        pthread_create(&customers[customers_created], NULL, customers_mainloop,
                (void*) customers_created);
        usleep(rand() % (2 * arrival_average));
        customers_created++;
    }

    for (i = 0; i < num_barbers; i++)
    {
        pthread_join(barbers[i], NULL);
    }

    clock_gettime(CLOCK_REALTIME, &stop);
    float elapsed = (stop.tv_sec - start.tv_sec) + 
        (stop.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("Time taken = %10.8f\n", elapsed);
}
