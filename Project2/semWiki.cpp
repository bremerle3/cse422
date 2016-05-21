#include <pthread.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <semaphore.h>
#include <vector>

#define DEBUG 1
//#define MIN_TIME 2
//#define MAX_THINK 10
//#define MAX_EAT 5 
#define MIN_TIME 1
#define MAX_THINK 2
#define MAX_EAT 2

int NUM_THREADS;
//pthread_mutex_t mut[NUM_THREADS];
//sem_t forkArray[NUM_THREADS];
sem_t* forkArray; // array of fork semaphore pointers

void *perform_work(void *argument)
{
	int passed_in_value;
	passed_in_value = *((int *) argument);
	while(true)
	{
		if(DEBUG)
			printf("%d Thinking\n", passed_in_value);
		int thinkTime = MIN_TIME + ( rand() % (MAX_THINK - MIN_TIME));
		if(DEBUG)
			printf("\tThink for %d secs\n", thinkTime);
		sleep(thinkTime);
		//pthread_mutex_lock(&mut[passed_in_value % NUM_THREADS]);
	        sem_wait(&forkArray[passed_in_value % NUM_THREADS]);  //Access the lower-numbered fork first.
		//pthread_mutex_lock(&mut[(passed_in_value + 1) % NUM_THREADS]); 
	        sem_wait(&forkArray[(passed_in_value + 1) % NUM_THREADS]);  //Then access the higher-numbered fork.
		if(DEBUG)
			printf("%d Eating\n", passed_in_value);
		int eatTime = MIN_TIME + ( rand() % (MAX_EAT - MIN_TIME));
		if(DEBUG)
			printf("\tEat for %d secs\n", eatTime);
		sleep(eatTime);
		//pthread_mutex_unlock(&mut[passed_in_value % NUM_THREADS]);
		sem_post(&forkArray[passed_in_value % NUM_THREADS]);
		//pthread_mutex_unlock(&mut[(passed_in_value + 1) % NUM_THREADS]);
		sem_post(&forkArray[(passed_in_value + 1) % NUM_THREADS]);
		if(DEBUG)
			printf("%d Finished Eating\n",passed_in_value);
	}
	return NULL;
}
 
int main(void)
{
    std::cout << "Enter number of philosophers from 1 to 15" << std::endl;
    std::cin >> NUM_THREADS;
    //Initialize forks as binary semaphores
    forkArray = new sem_t[NUM_THREADS];
    for(int idx=0; idx<NUM_THREADS; idx++)
    {

        sem_init(&forkArray[idx], 0, 1);
    }
    //Set up the threads
    pthread_t threads[NUM_THREADS];
    int thread_args[NUM_THREADS];
    int result_code, index;
    // create all threads one by one
    for (index = 0; index < NUM_THREADS; ++index) {
        thread_args[index] = index;
        if(DEBUG)
            printf("In main: creating thread %d\n", index);
        result_code = pthread_create(&threads[index], NULL, perform_work, (void *) &thread_args[index]);
        assert(0 == result_code);
    }
    // wait for each thread to complete
    for (index = 0; index < NUM_THREADS; ++index) {
        // block until thread 'index' completes
        result_code = pthread_join(threads[index], NULL);
        if(DEBUG)
            printf("In main: thread %d has completed\n", index);
        assert(0 == result_code);
    } 
    printf("In main: All threads completed successfully\n");
    exit(EXIT_SUCCESS);
}


