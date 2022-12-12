#include "helper.h"

using namespace std;

// error codes from sem_* functions stashed in errno
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
 * integer value ('value') and store the address in 'sem'. Output an error
 * message and exit if unsuccessful. */
void semaphore_init(sem_t *sem, unsigned int value) {
    if (sem_init(sem, 0, value) == -1) {
        perror("[Error initialising semaphore]");
        exit(1);
    }
}

/* Wrapper around sem_destroy() to destroy a semaphore at the address pointed to
 * by 'sem'. Output an error message if unsuccessful. */
void semaphore_destroy(sem_t *sem) {
    if (sem_destroy(sem) == -1) {
        perror("[Error destroying semaphore]");
    }
}

/* Iteratively join threads based on IDs in 'threads' array of size 'nthreads'.
 * Output an error message if unsuccessful. */
void join_threads(pthread_t *threads, const int &nthreads) {
    for (int t = 0; t < nthreads; t++) {
        int rc = pthread_join(threads[t], NULL);
        // error code itself is returned, not stashed in errno
        if (rc) {
            cerr << "[Error joining thread] pthread_join() failed with return "
                    "code: "
                 << rc << endl;
        }
    }
}