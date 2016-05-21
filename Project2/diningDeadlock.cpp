#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_DINERS 2

void* action(void*);

semt_t fork[NUM_DINERS];
int n;

void main()
{
    //Initialize forks as binary semaphores
    for(int idx=0; idx<NUM_DINERS; idx++)
    {
        sem_init(fork[idx], 0, 1);
    }
    //Each philosopher gets their own thread
    pthread_t philosophers[NUM_DINERS];
    //Create all of the philosopher threads
    int i,j,k;
    i = 0;
    j = 0;
    n = NUM_DINERS;
    for(i=0; i<n; i++)
    {
        j=i;
        pthread_create(&philosophers[i], NULL, action,&j);
    }
    //Join the threads
    for(j=0; j<n; j++)
    {
        pthread_join(&philosophers[i], NULL);
    }
}

void *action(void* arg)
{
  int val = *(int*)arg;
  printf("%d Thinking\n", val + 1);
  pthread_mutex_lock(&mut[val % n]);
  pthread_mutex_lock(&mut[(val + 1) % n]);
  printf("%d Eating\n", val + 1);
  sleep(3);
  pthread_mutex_unlock(&mut[val % n]);
  pthread_mutex_unlock(&mut[(val + 1) % n]);
  printf("%d Finished Eating\n",val + 1);
  pthread_exit(0);
}

