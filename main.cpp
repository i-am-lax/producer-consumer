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

/* Producer function declaration:
- ... */
void *producer(void *id);

/* Consumer function declaration:
- ... */
void *consumer(void *id);

// global variable to be shared across threads
Buffer b;

int main(int argc, char **argv) {
    // read in command-line arguments
    int qsize = check_arg(argv[1]);
    int njobs = check_arg(argv[2]);
    int nproducers = check_arg(argv[3]);
    int nconsumers = check_arg(argv[4]);

    // populate global Buffer 'b'
    b.queue = boost::circular_buffer<Job>(qsize);
    b.njobs = njobs;
    b.free = create_semaphore("/free", qsize);
    b.occupied = create_semaphore("/occupied", 0);
    b.mutex = create_semaphore("/mutex", 1);

    // Create producers based on nproducers
    pthread_t *pthreads = create_threads(nproducers, producer);
    // Create consumers based on nconsumers
    pthread_t *cthreads = create_threads(nconsumers, consumer);

    // Join producers
    for (int p = 0; p < nproducers; p++) {
        cout << "THREAD CONTENTS: " << pthreads[p] << endl;
        pthread_join(pthreads[p], NULL);
        cout << "Joining producer thread: " << p << endl;
        // TODO: error handling
    }

    // Join consumers
    for (int c = 0; c < nconsumers; c++)
    {
      cout << "THREAD CONTENTS: " << pthreads[p] << endl;
      pthread_join(pthreads[c], NULL);
      cout << "Joining consumer thread: " << c << endl;
    }

    // close named semaphores
    close_semaphore(b.free);
    close_semaphore(b.occupied);
    close_semaphore(b.mutex);

    return 0;
}

// TODO: add 20s timeout
void *producer(void *id) {
    // producer ID
    int *pid = (int *) id;

    // // given producer creates up maximum of 'njobs'
    // for (int j = 0; j < b.njobs; j++) {
    //     // create job every 1 - 5 seconds
    //     sleep(rand() % 5 + 1);

    //     /* initiate locks -
    //     - adding a job would decrement free slots available in queue
    //     - mutex decremented so only one producer or consumer at a time */
    //     sem_wait(b.free);
    //     sem_wait(b.mutex);

    //     // create job and add to the queue
    //     Job job = {b.queue.size(), rand() % 10 + 1};
    //     b.queue.push_back(job);
    //     cout << "Producer(" << *pid << "): Job id " << job.id << " duration "
    //          << job.duration << endl;

    //     /* release locks -
    //     - mutex incremented so producer or consumer can execute
    //     - occupied incremented to signal consumer to process job */
    //     sem_post(b.mutex);
    //     sem_post(b.occupied);
    // }

    pthread_exit(0);
}

// TODO: add while loop and 20s timeout
void *consumer(void *id) {
    // consumer ID
    int *cid = (int *) id;

    // /* initiate locks - */
    // sem_wait(b.occupied);
    // sem_wait(b.mutex);

    // // take job from front of queue
    // Job job = b.queue[0];
    // b.queue.pop_front();
    // cout << "Consumer(" << *cid << "): Job id " << job.id << " executing
    // sleep duration "
    //       << job.duration << endl;

    // /* release locks - */
    // sem_post(b.mutex);
    // sem_post(b.free);

    // // process job
    // sleep(job.duration);
    // cout << "Consumer(" << *cid << "): Job id " << job.id << " completed" <<
    // endl;

    pthread_exit(0);
}
