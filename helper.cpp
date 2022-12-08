#include "helper.h"

extern int errno;

/* Check if a given command-line input ('buffer') is an integer and if so,
 * return it. */
int check_arg(char *buffer) {
    int i, num = 0, temp = 0;
    if (strlen(buffer) == 0)
        return -1;
    for (i = 0; i < (int) strlen(buffer); i++) {
        temp = 0 + buffer[i];
        if (temp > 57 || temp < 48)
            return -1;
        num += pow(10, strlen(buffer) - i - 1) * (buffer[i] - 48);
    }
    return num;
}

/* Wrapper around sem_init() to initialise an unnamed semaphore with an
 * integer value ('value') and store the address in 'sem'. Return true if
 * successful otherwise output an error message and return false. */
bool semaphore_init(sem_t *sem, unsigned int value) {
    if (sem_init(sem, 0, value) == -1) {
        perror("[Error initialising semaphore]");
        return false;
    }
    return true;
}

/* Wrapper around sem_destroy() to destroy a semaphore at the address pointed to
 * by 'sem'. Return true if successful otherwise output an error message and
 * return false. */
bool semaphore_destroy(sem_t *sem) {
    if (sem_destroy(sem) == -1) {
        perror("[Error destroying semaphore]");
        return false;
    }
    return true;
}

/* Iteratively join threads based on IDs in 'threads' array of size 'nthreads'.
 * Return true if successful otherwise output error message and return false. */
bool join_threads(pthread_t *threads, const int &nthreads) {
    for (int t = 0; t < nthreads; t++) {
        int rc = pthread_join(threads[t], NULL);
        // error code itself is returned, not stashed in errno
        if (rc) {
            cerr << "[Error joining thread] pthread_join() failed with return "
                    "code: "
                 << rc << endl;
            return false;
        }
    }
    return true;
}