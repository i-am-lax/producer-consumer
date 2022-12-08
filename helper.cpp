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

/* Wrapper around sem_init() to create an unnamed semaphore with an initial
 * integer value ('value') and store the address in 'sem'. Return true if
 * successful otherwise output an error message and return false. */
bool create_semaphore(sem_t *sem, unsigned int value) {
    int rc = sem_init(sem, 0, value);
    if (rc == -1) {
        cerr << "[Error] sem_open() failed to create named semaphore '"
             << "' with errno: " << errno << endl;
        return false;
    }
    return true;
}

/* Wrapper around sem_destroy() to destroy a semaphore at the address pointed to
 * by 'sem'. Return true if successful otherwise output an error message and
 * return false. */
bool destroy_semaphore(sem_t *sem) {
    if (sem_destroy(sem) == -1) {
        cerr << "[Error] sem_close() failed with errno: " << errno << endl;
        return false;
    }
    return true;
}

/* Iteratively join threads based on IDs in 'threads' array of size 'nthreads'.
 * Return true if successful otherwise output an error message and return false.
 */
bool join_threads(pthread_t *threads, int &nthreads) {
    for (int t = 0; t < nthreads; t++) {
        int rc = pthread_join(threads[t], NULL);
        if (rc) {
            cerr
                << "[Error] pthread_join() for thread failed with return code: "
                << rc << endl;
            return false;
        }
    }
    return true;
}