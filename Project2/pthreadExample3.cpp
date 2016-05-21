#include<stdio.h>
#include<cstdlib>
#include<stdlib.h>
#include<pthread.h>
#include <unistd.h>
#include<iostream>

#define NUM_PHILOSOPHERS 5

void* func(int n);
//void* func(void* n);
pthread_t philosopher[NUM_PHILOSOPHERS];
pthread_mutex_t chopstick[NUM_PHILOSOPHERS];

void * functional1(void* argument);

int main()
{
     int i;
     void *msg;
     for(i=1;i<=NUM_PHILOSOPHERS;i++)
     {
          pthread_mutex_init(&chopstick[i],NULL);
     }
     for(i=1;i<=NUM_PHILOSOPHERS;i++)
     {
          //pthread_create(&philosopher[i],NULL,(void *)func,(int *)i);
          //pthread_create(&philosopher[i],NULL,func,(void*)i);
	  int * idx;
	  *idx = i;
          pthread_create(&philosopher[i],NULL,functional1,(void*)idx);
     }
     for(i=1;i<=NUM_PHILOSOPHERS;i++)
     {
          pthread_join(philosopher[i],&msg);
     }
      for(i=1;i<=NUM_PHILOSOPHERS;i++)
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

void * functional1(void * argument)
{
	int num = *((int* )argument);
	std::cout << "thread " << num << std::endl;
	return 0;
}
