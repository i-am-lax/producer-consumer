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
#include <time.h>

/* Job data structure. Producer creates a job in a queue which is then processed
by a consumer. Job is made up of -
- id (the location that it occupies in the queue)
- duration (the time taken to process the job in seconds) */
struct Job {
    int id;
    int duration;
};

/* Buffer data structure (to be shared across threads) consists of:
 * queue -> circular buffer with n slots of type 'Job' (read/write)
 * free -> pointer to semaphore representing free space in queue
 * occupied -> pointer to semaphore representing presence of jobs in queue
 * mutex -> pointer to semaphore ensuring mutual exclusivity between producers /
 * consumers
 * njobs -> number of jobs per producer (read-only) */
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

/* Initialise an unnamed semaphore with an integer value ('value') and store
 * the address in 'sem'. Exit if unsuccessful. */
void semaphore_init(sem_t *sem, unsigned int value);

/* Destroy an unnamed semaphore at the address pointed to by 'sem'. Exit if
 * unsuccessful. */
void semaphore_destroy(sem_t *sem);

/* Iteratively join threads based on IDs in 'threads' array of size 'nthreads'.
 * Exit if unsuccessful. */
void join_threads(pthread_t *threads, const int &nthreads);