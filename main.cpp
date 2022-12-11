/******************************************************************
 Main program.
 Supply 4 command-line arguments:
 - size of the queue
 - number of jobs to generate for each producer
 - number of producers
 - number of consumers
 Example execution: ./main 5 6 2 3
 ******************************************************************/

#include "helper.h"

using namespace std;

// Global Buffer to be shared across threads
Buffer b;

/* Global timeout constant to represent max wait time for both producers and
 * consumers before terminating thread */
const int timeout = 20;

/* Producer routine takes in a unique 'id' to represent the producer. The
 * producer creates a maximum of n jobs every 1 to 5 seconds and adds them to a
 * circular queue. */
void *producer(void *id);

/* Consumer routine takes in a unique 'id' to represent the consumer. The
 * consumer takes jobs from the front of the queue and processes them. */
void *consumer(void *id);

int main(int argc, char **argv) {

    // Ensure that 4 input arguments supplied else output error and exit
    if (argc != 5) {
        cerr << "[Error] Supply 4 arguments: queue size, number of jobs per "
                "producer, number of producers, number of consumers"
             << endl;
        exit(1);
    }

    // Validate command-line arguments and initialise variables
    int qsize = check_arg(argv[1]);
    int njobs = check_arg(argv[2]);
    int nproducers = check_arg(argv[3]);
    int nconsumers = check_arg(argv[4]);

    /* Populate Buffer data structure 'b' to be shared across threads
     * queue -> circular buffer with slots 'qsize' and of type 'Job'
     * njobs -> number of jobs per producer given by 'njobs'
     * ts -> store maximum waiting time for producers and consumers
     * free -> semaphore (initial value 'qsize') for free slots in 'queue'
     * occupied -> semaphore (initial value 0) for whether or not a job is
     * available in 'queue'
     * mutex -> semaphore (initial value 1) so that access to the shared queue
     * is mutually exclusive */
    b.queue = boost::circular_buffer<Job>(qsize);
    b.njobs = njobs;
    b.ts.tv_sec = time(NULL) + timeout;
    // create semaphores and assign to pointers in buffer 'b'
    sem_t free, occupied, mutex;
    semaphore_init(&free, qsize);
    semaphore_init(&occupied, 0);
    semaphore_init(&mutex, 1);
    b.free = &free;
    b.occupied = &occupied;
    b.mutex = &mutex;

    /* Declare arrays:
     * pids -> producer IDs
     * cids -> consumer IDs
     * pthreads -> thread IDs for producers
     * cthreads -> thread IDs for consumers */
    pthread_t pthreads[nproducers], cthreads[nconsumers];
    int pids[nproducers], cids[nconsumers];

    /* Iteratively create producer threads based on 'nproducers' and execute
     * 'producer' on each. Store the thread ID in 'pthreads' and producer ID in
     * 'pids'. Incase of failure, output an error message with code. */
    for (int n = 0; n < nproducers; n++) {
        pids[n] = n + 1;
        int rc =
            pthread_create(&pthreads[n], NULL, producer, (void *) &pids[n]);
        if (rc) {
            cerr << "[Error creating thread] pthread_create() for Producer("
                 << n + 1 << ") failed with error code: " << rc << endl;
            exit(1);
        }
    }

    /* Create consumer threads based on 'nconsumers' in the same manner as for
     * producers. */
    for (int n = 0; n < nconsumers; n++) {
        cids[n] = n + 1;
        int rc =
            pthread_create(&cthreads[n], NULL, consumer, (void *) &cids[n]);
        if (rc) {
            cerr << "[Error creating thread] pthread_create() for Consumer("
                 << n + 1 << ") failed with error code: " << rc << endl;
            exit(1);
        }
    }

    // Join producer and consumer threads so we wait for them to finish
    join_threads(pthreads, nproducers);
    join_threads(cthreads, nconsumers);

    // Destroy semaphores
    semaphore_destroy(b.free);
    semaphore_destroy(b.occupied);
    semaphore_destroy(b.mutex);

    return 0;
}

/* Routine for each producer thread:
 * - Producer identified with unique 'id'
 * - Create a job every 1 - 5 seconds (up to a maximum number of jobs based on
 * user input) and add to the circular queue
 * -  Duration for each job is between 1 – 10 seconds
 * - If a job is taken by the consumer, then another job can be produced which
 * has the same id
 * - If queue is full, block while waiting for an empty slot until timeout
 * reached upon which thread terminated
 * - Otherwise thread terminated when all jobs produced */
void *producer(void *id) {
    // producer ID
    int *pid = (int *) id;

    // declare job
    Job job;

    // given producer creates up to maximum of 'njobs'
    for (int j = 0; j < b.njobs; j++) {

        // create job every 1 - 5 seconds
        sleep(rand() % 5 + 1);

        /* initiate locks -
        - adding a job would decrement free slots available in queue
        - mutex decremented so only one producer or consumer at a time */
        if (sem_timedwait(b.free, &b.ts) == -1 && errno == ETIMEDOUT) {
            // exit if blocking time exceeds timeout
            cout << "Producer(" << *pid << "): Timeout after 20 seconds"
                 << endl;
            pthread_exit(0);
        }
        if (sem_wait(b.mutex) == -1) {
            perror("[Error locking semaphore 'mutex']");
            pthread_exit(0);
        }

        // critical section: create job and add to the queue
        job = {(int) b.queue.size(), rand() % 10 + 1};
        b.queue.push_back(job);

        /* release locks -
        - mutex incremented so producer / consumer can execute
        - occupied incremented to signal consumer to process job */
        if (sem_post(b.mutex) == -1) {
            perror("[Error unlocking semaphore 'mutex']");
            pthread_exit(0);
        }
        if (sem_post(b.occupied) == -1) {
            perror("[Error unlocking semaphore 'occupied']");
            pthread_exit(0);
        }

        cout << "Producer(" << *pid << "): Job id " << job.id << " duration "
             << job.duration << endl;
    }
    cout << "Producer(" << *pid << "): No more jobs to generate." << endl;

    pthread_exit(0);
}

/* Routine for each consumer thread:
 * - Consumer identified with unique 'id'
 * - Take a job from the circular queue and sleep for the specified duration
 * - If there are no jobs in the queue, wait until timeout reached upon
 * which thread is terminated. */
void *consumer(void *id) {
    // consumer ID
    int *cid = (int *) id;

    // declare job
    Job job;

    while (true) {

        /* initiate locks -
        - consuming a job would decrement number of jobs occupying queue
        - mutex decremented so only one producer / consumer at a time */
        if (sem_timedwait(b.occupied, &b.ts) == -1 && errno == ETIMEDOUT) {
            // exit if blocking time exceeds timeout
            cout << "Consumer(" << *cid << "): No more jobs left." << endl;
            pthread_exit(0);
        }
        if (sem_wait(b.mutex) == -1) {
            perror("[Error locking semaphore 'mutex']");
            pthread_exit(0);
        }

        // critical section: take job from front of queue
        job = b.queue[0];
        b.queue.pop_front();

        /* release locks -
        - mutex incremented so producer / consumer can execute
        - free slots available incremented as job has been removed from queue */
        if (sem_post(b.mutex) == -1) {
            perror("[Error unlocking semaphore 'mutex']");
            pthread_exit(0);
        }
        if (sem_post(b.free) == -1) {
            perror("[Error unlocking semaphore 'free']");
            pthread_exit(0);
        }

        cout << "Consumer(" << *cid << "): Job id " << job.id
             << " executing sleep duration " << job.duration << endl;

        // process job
        sleep(job.duration);
        cout << "Consumer(" << *cid << "): Job id " << job.id << " completed"
             << endl;
    }
}
