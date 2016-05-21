#include<stdio.h>
#include<cstdlib>
#include<stdlib.h>
#include<pthread.h>
#include <unistd.h>

void* func(int n);
//void* func(void* n);
pthread_t philosopher[5];
pthread_mutex_t chopstick[5];

int main()
{
     int i;
     void *msg;
     for(i=1;i<=5;i++)
     {
          pthread_mutex_init(&chopstick[i],NULL);
     }
     for(i=1;i<=5;i++)
     {
          //pthread_create(&philosopher[i],NULL,(void *)func,(int *)i);
          pthread_create(&philosopher[i],NULL,func,(void*)i);
     }
     for(i=1;i<=5;i++)
     {
          pthread_join(philosopher[i],&msg);
     }
      for(i=1;i<=5;i++)
      {
          pthread_mutex_destroy(&chopstick[i]);
      }
     return 0;
}

void* func(int n)
//void* func(void * n)
{
	

     printf ("\nPhilosopher %d is thinking ",n);
     pthread_mutex_lock(&chopstick[n]);//when philosopher 5 is eating he takes fork 1 and fork 5
     pthread_mutex_lock(&chopstick[(n+1)%5]);
     printf ("\nPhilosopher %d is eating ",n);
     sleep(3);
     pthread_mutex_unlock(&chopstick[n]);
     pthread_mutex_unlock(&chopstick[(n+1)%5]);
     printf ("\nPhilosopher %d finished eating ",n);
}
