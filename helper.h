/******************************************************************
 * Header file for the helper functions. This file includes the
 * required header files, function signatures and custom data
 * structures.
 ******************************************************************/

#include "semaphore.h"
#include <boost/circular_buffer.hpp>
#include <errno.h>
#include <iostream>
#include <math.h>
#include <pthread.h>
#include <string.h>

using namespace std;

/* Job data structure. Producer creates a job in a queue which is then processed
by a consumer. Job is made up of -
- id (the location that it occupies in the queue)
- duration (the time taken to process the job in seconds) */
struct Job {
    int id;
    int duration;
};

/* Buffer data structure consists of:
 * queue -> circular buffer with slots 'qsize' and of type 'Job'
 * njobs -> number of jobs per producer
 * free -> represents free space in queue
 * occupied -> represents presence of jobs in queue
 * mutex -> ensures mutual exclusivity between producers and consumers */
struct Buffer {
    boost::circular_buffer<Job> queue;
    sem_t *free;
    sem_t *occupied;
    sem_t *mutex;
    int njobs;
};

/* Check if the characters in a given command-line input ('buffer') can be
 * represented as an integer. If so, then return it. */
int check_arg(char *buffer);

/* Create an unnamed semaphore with an initial integer value ('value') and store
 * the address in 'sem'. Return true if successful. */
bool create_semaphore(sem_t *sem, unsigned int value);

/* Destroy an unnamed semaphore at the address pointed to by 'sem'.  Return true
 * if successful. */
bool destroy_semaphore(sem_t *sem);

/* Iteratively join threads based on IDs in 'threads' array of size 'nthreads'.
 * Return true if successful. */
bool join_threads(pthread_t *threads, int &nthreads);