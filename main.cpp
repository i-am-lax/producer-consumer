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
#include "semaphore.h"
#include <boost/circular_buffer.hpp>

using namespace std;

/* Job is made up of -
- id (the location that it occupies in the queue)
- duration (the time taken to process the job in seconds) */
struct Job {
    int id;
    int duration;
};

/* Buffer comprises of the circular queue, total jobs per producer, and pointers
to semaphores to be passed to the producers and consumers. Semaphores:
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
    int qsize = check_arg(argv[2]);
    int njobs = check_arg(argv[1]);
    int nproducers = check_arg(argv[3]);
    int nconsumers = check_arg(argv[4]);

    // create array of thread IDs for producers and consumers
    pthread_t pthreads[nproducers], cthreads[nconsumers];

    // populate global Buffer 'b'
    b.queue = boost::circular_buffer<Job>(qsize);
    b.njobs = njobs;
    b.free = sem_open("/free", O_CREAT, 0644, qsize);
    b.occupied = sem_open("/occupied", O_CREAT, 0644, 0);
    b.mutex = sem_open("/mutex", O_CREAT, 0644, 1);

    // TODO: fn to create semaphore where you supply name and error checking
    // done
    if (b.free == (void *) -1) {
        perror("sem_open() failed for semfree");
    }

    // store output of thread creation and joining for error handling
    int pc;

    // Create producers based on nproducers
    for (int p = 0; p < nproducers; p++) {
        cout << "Creating producer thread: " << p << endl;
        rc = pthread_create(&pthreads[p], NULL, producer, (void *) &p);
        // TODO: error handling
    }

    // Create consumers based on nconsumers
    for (int c = 0; c < nconsumers; c++)
    {
      cout << "Creating consumer thread: " << c << endl;
      rc = pthread_create(&cthreads[c], NULL, consumer, (void *)&c);
    }

    // Join producers
    for (int p = 0; p < nproducers; p++) {
        rc = pthread_join(pthreads[p], NULL);
        cout << "Joining producer thread: " << p << endl;
        // TODO: error handling
    }

    // Join consumers
    for (int c = 0; c < nconsumers; c++)
    {
      rc = pthread_join(pthreads[c], NULL);
      cout << "Joining consumer thread: " << c << endl;
    }

    cout << "Destroy semaphores!!" << endl;
    sem_close(b.free);
    sem_close(b.occupied);
    sem_close(b.mutex);
    
    return 0;
}

// TODO: add 20s timeout
void *producer(void *id) {
    // producer ID
    int *pid = (int *) id;

    // given producer creates up maximum of 'njobs'
    for (int j = 0; j < b.njobs; j++) {
        // create job every 1 - 5 seconds
        sleep(rand() % 5 + 1);

        /* initiate locks -
        - adding a job would decrement free slots available in queue
        - mutex decremented so only one producer or consumer at a time */
        sem_wait(b.free);
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

    /* initiate locks - */
    sem_wait(b.occupied);
    sem_wait(b.mutex);

    // take job from front of queue
    Job job = b.queue[0];
    b.queue.pop_front();
    cout << "Consumer(" << *cid << "): Job id " << job.id << " executing sleep duration "
          << job.duration << endl;

    /* release locks - */
    sem_post(b.mutex);
    sem_post(b.free);
    
    // process job
    sleep(job.duration);
    cout << "Consumer(" << *cid << "): Job id " << job.id << " completed" << endl;

    pthread_exit(0);
}
