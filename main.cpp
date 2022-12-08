/******************************************************************
 Main program.
 Supply 4 command-line arguments:
 - size of the queue
 - number of jobs to generate for each producer
 - number of producers
 - number of consumers
 Example execution: ./main 5 6 2 3

 Please note that MacOS doesn’t support unnamed semaphores
 (sem_init() and sem_destroy()), only named semaphores
 (sem_open()) and sem_close()).
 ******************************************************************/

#include "helper.h"

using namespace std;

/* Producer function takes in a unique 'id' to represent the producer. The
 * producer creates a maximum of n jobs every 1 to 5 seconds and adds them to a
 * circular queue. */
void *producer(void *id);

/* Consumer function takes in a unique 'id' to represent the consumer. The
 * consumer takes jobs from the front of the queue and processes them. */
void *consumer(void *id);

/* Join threads given by IDs in 'threads' array with size 'nthreads' */
bool join_threads(pthread_t *threads, int &nthreads);

// Global Buffer variable to be shared across threads
Buffer b;
const int timeout = 20;

// struct timespec ts;
// ts.tv_sec = time(NULL) + timeout;

int main(int argc, char **argv) {

    // Ensure that 4 input arguments supplied else output error and exit
    if (argc != 5) {
        cerr << "[Error] Supply 4 arguments: queue size, number of jobs per "
                "producer, number of producers, number of consumers"
             << endl;
        exit(1);
    }
    cout << "Passed arg check.." << endl;

    // Validate command-line arguments and initialise variables
    int qsize = check_arg(argv[1]);
    int njobs = check_arg(argv[2]);
    int nproducers = check_arg(argv[3]);
    int nconsumers = check_arg(argv[4]);

    cout << "Read in arguments." << endl;

    /* Populate Buffer data structure 'b' to be shared across threads
     * queue -> circular buffer with slots 'qsize' and of type 'Job'
     * njobs -> number of jobs per producer given by 'njobs'
     * free -> semaphore (initial value 'qsize') for free slots in 'queue'
     * occupied -> semaphore (initial value 0) for whether or not a job is
     * available in 'queue'
     * mutex -> semaphore (initial value 1) to represent mutual exclusivity */
    b.queue = boost::circular_buffer<Job>(qsize);
    b.njobs = njobs;
    cout << "Initialised queue and njobs" << endl;
    sem_t free, occupied, mutex;
    create_semaphore(&free, qsize);
    create_semaphore(&occupied, 0);
    create_semaphore(&mutex, 1);

    b.free = &free;
    b.occupied = &occupied;
    b.mutex = &mutex;

    /* Initisalise arrays:
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
        pids[n] = n;
        int ret =
            pthread_create(&pthreads[n], NULL, producer, (void *) &pids[n]);
        if (ret) {
            cerr << "[Error] pthread_create() for Producer(" << n
                 << ") failed with return code: " << ret << endl;
            exit(1);
        }
    }

    /* Create consumer threads based on 'nconsumers' in the same manner as for
     * producers. */
    for (int n = 0; n < nconsumers; n++) {
        cids[n] = n;
        int ret =
            pthread_create(&cthreads[n], NULL, consumer, (void *) &cids[n]);
        if (ret) {
            cerr << "[Error] pthread_create() for Consumer(" << n
                 << ") failed with return code: " << ret << endl;
            exit(1);
        }
    }

    // Join producer and consumer threads so we wait for them to finish
    join_threads(pthreads, nproducers);
    join_threads(cthreads, nconsumers);

    // close named semaphores
    cout << "close semaphores!" << endl;
    destroy_semaphore(b.free);
    destroy_semaphore(b.occupied);
    destroy_semaphore(b.mutex);

    return 0;
}

bool join_threads(pthread_t *threads, int &nthreads) {
    for (int t = 0; t < nthreads; t++) {
        int ret = pthread_join(threads[t], NULL);
        if (ret) {
            cerr
                << "[Error] pthread_join() for thread failed with return code: "
                << ret << endl;
        }
    }
    return true;
}

void *producer(void *id) {

    // producer ID
    int *pid = (int *) id;

    // given producer creates up to maximum of 'njobs'
    for (int j = 0; j < b.njobs; j++) {

        // create job every 1 - 5 seconds
        sleep(rand() % 5 + 1);

        // TODO: do not print or anything during lock phase

        /* initiate locks -
        - adding a job would decrement free slots available in queue
        - mutex decremented so only one producer or consumer at a time */
        struct timespec ts;
        ts.tv_sec = time(NULL) + timeout;
        if (sem_timedwait(b.free, ts) == -1) {
            cout << "Producer(" << *pid << "): Timeout after 20 seconds"
                 << endl;
            pthread_exit(0);
        }
        sem_wait(b.mutex);

        // create job and add to the queue
        Job job = {b.queue.size(), rand() % 10 + 1};
        b.queue.push_back(job);
        cout << "Producer(" << *pid << "): Job id " << job.id << " duration "
             << job.duration << endl;

        /* release locks -
        - mutex incremented so producer or consumer can execute
        - occupied incremented to signal consumer to process job */
        sem_post(b.mutex);
        sem_post(b.occupied);
    }

    pthread_exit(0);
}

// TODO: add while loop and 20s timeout
void *consumer(void *id) {
    // consumer ID
    int *cid = (int *) id;

    // TODO: do not print or anything during lock phase

    /* initiate locks - */
    struct timespec ts;
    ts.tv_sec = time(NULL) + timeout;
    if (sem_timedwait(b.occupied, ts) == -1) {
        cout << "Consumer(" << *cid << "): Timeout after 20 seconds" << endl;
        pthread_exit(0);
    }
    sem_wait(b.mutex);

    // take job from front of queue
    Job job = b.queue[0];
    b.queue.pop_front();
    cout << "Consumer(" << *cid << "): Job id " << job.id
         << " executing sleep duration " << job.duration << endl;

    /* release locks - */
    sem_post(b.mutex);
    sem_post(b.free);

    // process job
    sleep(job.duration);
    cout << "Consumer(" << *cid << "): Job id " << job.id << " completed"
         << endl;

    pthread_exit(0);
}
