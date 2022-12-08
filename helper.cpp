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

/* Wrapper around sem_init (with only inputs 'name' and 'value') to create named
 * semaphore and exit incase of failure. */
bool create_semaphore(sem_t *sem, unsigned int value) {
    errno = 0;
    int rc = sem_init(sem, 1, value);
    if (s == -1) {
        cerr << "[Error] sem_open() failed to create named semaphore '" << name
             << "' with errno: " << errno << endl;
        exit(1);
    }
    return true;
}

/* Wrapper around sem_destroy to destroy a semaphore pointed to by 'sem'. If
this is unsuccessful an error message is printed. */
void destroy_semaphore(sem_t *sem) {
    errno = 0;
    if (sem_destroy(sem) == -1) {
        cerr << "[Error] sem_close() failed with errno: " << errno << endl;
    }
}