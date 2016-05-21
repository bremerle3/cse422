#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

#define DEBUG 1
#define NUM_THREADS 15
//#define MIN_TIME 2
//#define MAX_THINK 10
//#define MAX_EAT 5
 
#define MIN_TIME 1
#define MAX_THINK 2
#define MAX_EAT 2

pthread_mutex_t mut[NUM_THREADS];

void *perform_work(void *argument)
{
	int passed_in_value;
	passed_in_value = *((int *) argument);
	while(true)
	{
		printf("%d Thinking\n", passed_in_value);
		int thinkTime = MIN_TIME + ( rand() % (MAX_THINK - MIN_TIME));
		if(DEBUG)
			printf("\tThink for %d secs\n", thinkTime);
		sleep(thinkTime);
		pthread_mutex_lock(&mut[passed_in_value % NUM_THREADS]);  //Access the lower-numbered fork first.
		pthread_mutex_lock(&mut[(passed_in_value + 1) % NUM_THREADS]);  //Then access the higher-numbered fork.
		printf("%d Eating\n", passed_in_value);
		int eatTime = MIN_TIME + ( rand() % (MAX_EAT - MIN_TIME));
		if(DEBUG)
			printf("\tEat for %d secs\n", eatTime);
		sleep(eatTime);
		pthread_mutex_unlock(&mut[passed_in_value % NUM_THREADS]);
		pthread_mutex_unlock(&mut[(passed_in_value + 1) % NUM_THREADS]);
		printf("%d Finished Eating\n",passed_in_value);
	}
	return NULL;
}
 
int main(void)
{
   pthread_t threads[NUM_THREADS];
   int thread_args[NUM_THREADS];
   int result_code, index;
 
   // create all threads one by one
   for (index = 0; index < NUM_THREADS; ++index) {
      thread_args[index] = index;
      printf("In main: creating thread %d\n", index);
      result_code = pthread_create(&threads[index], NULL, perform_work, (void *) &thread_args[index]);
      assert(0 == result_code);
   }
 
   // wait for each thread to complete
   for (index = 0; index < NUM_THREADS; ++index) {
      // block until thread 'index' completes
      result_code = pthread_join(threads[index], NULL);
      printf("In main: thread %d has completed\n", index);
      assert(0 == result_code);
   }
 
   printf("In main: All threads completed successfully\n");
   exit(EXIT_SUCCESS);
}

