/******************************************************************
 * Header file for the helper functions. This file includes the
 * required header files, function signatures and custom data
 * structures.
 ******************************************************************/

// # include <stdio.h>
// # include <stdlib.h>
// # include <unistd.h>
// # include <sys/types.h>
// # include <sys/ipc.h>
// # include <sys/shm.h>
// # include <sys/sem.h>
// # include <sys/time.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <string.h>
// # include <ctype.h>
#include "semaphore.h"
#include <boost/circular_buffer.hpp>
#include <iostream>

using namespace std;

/* Job data structure. Producer creates a job in a queue which is then processed
by a consumer. Job is made up of -
- id (the location that it occupies in the queue)
- duration (the time taken to process the job in seconds) */
struct Job {
    int id;
    int duration;
};

/* Buffer data structure. Buffer comprises of the circular queue, total jobs per
producer, and pointers to semaphores to be passed to the producers and
consumers. Semaphores:
- free -> represents free space in queue
- occupied -> represents presence of jobs in queue
- mutex -> ensures mutual exclusion between producers and consumers */
struct Buffer {
    boost::circular_buffer<Job> queue;
    sem_t *free;
    sem_t *occupied;
    sem_t *mutex;
    int njobs;
};

// Check if a command-line input is an integer and return it
int check_arg(char *);

// Create a named sempahore ('name') with an initial integer value ('value').
// Return a pointer to the semaphore.
sem_t *create_semaphore(const char *name, unsigned int value);

// Close a named sempahore pointed to by 'sem'
void close_semaphore(sem_t *sem);

/* Create 'nthreads' number of threads and execute 'start_routine'. Return a
 * pointer to an array containing the thread IDs. */
// pthread_t *create_threads(int nthreads, void *(*start_routine)(void *) );
