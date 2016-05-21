#include<stdio.h>
#include<pthread.h>
#include <unistd.h>

#define NUM_DINERS 5
void* reader(void*);
pthread_mutex_t mut[NUM_DINERS];
int n;
 
int main()
{
  pthread_t t1[NUM_DINERS];
  int i,j = 0,k;
  n = NUM_DINERS;
  for(i = 0; i < n; i++)
    {
      j=i;
      pthread_create(&t1[i], NULL, reader, &j);
    }
  for(j = 0; j < n; j++)
    pthread_join(t1[j], NULL);
  return 1;
}
 
void *reader(void* arg)
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
}
