#include "helper.h"

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

/* Wrapper around sem_open (with only inputs 'name' and 'value') to create named
 * semaphore and exit incase of failure. */
sem_t *create_semaphore(const char *name, unsigned int value) {
    errno = 0;
    sem_t *s = sem_open(name, O_CREAT, 0644, value);
    if (s == SEM_FAILED) {
        cerr << "[Error] sem_open() failed to create named semaphore '" << name
             << "' with errno: " << errno << endl;
        exit(1);
    }
    return s;
}

/* Wrapper around sem_close to close a named sempahore pointed to by 'sem'. If
this is unsuccessful an error message is printed. */
void close_semaphore(sem_t *sem) {
    errno = 0;
    if (sem_close(sem) == -1) {
        cerr << "[Error] sem_close() failed with errno: " << errno << endl;
    }
}

/* Wrapped around pthread_create() to iteratively create 'nthreads' number of
 * threads and execute 'start_routine'. Incase of failure, an error message is
 * printed and we exit. If successful, we return a pointer to an array containing
 * the thread IDs. */
pthread_t *create_threads(int nthreads, void *(*start_routine)(void *) ) {
    int ret;
    pthread_t threads[nthreads];
    for (int n = 0; n < nthreads; n++) {
        cout << "Creating thread: " << n << endl;
        ret = pthread_create(&threads[n], NULL, start_routine, (void *) &n);
        if (ret) {
            cerr << "[Error] pthread_create() failed with return code: " << ret
                 << endl;
            exit(1);
        }
    }
    return threads;
}