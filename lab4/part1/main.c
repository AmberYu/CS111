#define _GNU_SOURCE

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <getopt.h>
#include <errno.h>
#include <math.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <pthread.h>
#include <time.h>

#define no_argument 0
#define required_argument 1 
#define optional_argument 2

#define ITER    'a'
#define THREADS    'b'
#define YIELD   'c'



static long long counter = 0;
static  int num_iterations;
static  int num_threads;
static int opt_yield = 0;
static pthread_mutex_t lock;

void add(long long *pointer, long long value) {
    long long sum = *pointer + value;
    if(opt_yield)
        pthread_yield();
    *pointer = sum;
}

void add_mutex(long long *pointer, long long value) {
    pthread_mutex_lock(&lock);
    long long sum = *pointer + value;
    *pointer = sum;
    pthread_mutex_unlock(&lock);
}

static volatile int test_lock = 0;
void add_spin_lock(long long *pointer, long long value) {
    while(__sync_lock_test_and_set(&test_lock,1));
    long long sum = *pointer + value;
    *pointer = sum;
    __sync_lock_release(&test_lock);
}

void add_atomic(long long *pointer, long long value) {
    long long orig;
    long long sum;
    do{
      orig = *pointer;     
      sum = orig + value;
    }while(__sync_val_compare_and_swap(pointer,orig,sum)!=orig);
}

void thread_func(){
  //for each thread, do some preparation work
  int i;
  for(i = 0; i < num_iterations; ++i){
    add_spin_lock(&counter,1);
  }
  for(i = 0; i < num_iterations; ++i){
    add_spin_lock(&counter,-1);
  }
}


int main(int argc, char * argv[])
{
  const struct option longopts[] =
  {
    {"iter",  optional_argument, 0, ITER },
    {"threads",  optional_argument, 0, THREADS },
    {"yield",  optional_argument, 0, YIELD },
    {0,           0,               0,  0   }  //denote end
  };

  //define time usage
  int iarg = 0;
  int index;
  errno = 0;

  
  while((iarg = getopt_long(argc, argv, "", longopts, &index)) != -1)
  {
      
    switch (iarg)
    {
      case ITER:
      {
        // printf("optarg:%s\n", optarg);
        if(optarg){
          // printf("optind:%d\n", optind);
          num_iterations = atoi(optarg);
        }
        else{
          num_iterations = 1;
        }
        // printf("iter:%d\n", num_iterations);
        break;

      }

      case THREADS:
      {
        if(optarg){
          num_threads = atoi(optarg);
        }
        else
          num_threads = 1;

        // printf("threads:%d\n", num_threads);
        break;
      }

      case YIELD:
      {
        if(optarg){
          opt_yield = 1;;
        }
        break;
      }
    }
  }
  struct timespec start,end;
  uint64_t diff;
  clock_gettime(CLOCK_MONOTONIC, &start);
  pthread_t *threads;
  threads = malloc(sizeof(pthread_t)*num_threads);
  int i;
  for( i = 0; i < num_threads; ++i){
    int ret = pthread_create(&threads[i], NULL,(void *) thread_func, NULL);
    if(ret != 0)
    {
      fprintf(stderr,"Failed with error [%s]\n",strerror(errno)); 
    }
  }

  for( i = 0; i < num_threads; ++i){
    int ret = pthread_join(threads[i],NULL);
    if(ret != 0)
    {
      fprintf(stderr,"Failed with error [%s]\n",strerror(errno)); 
    }
  }

  clock_gettime(CLOCK_MONOTONIC, &end);
  diff = 1000000000 * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;

printf("%d threads * %d iterations * (add + substract) =  %d operations\n", num_threads, num_iterations, (num_threads * num_iterations * 2));
printf("Final count = %lld\n", counter);
printf("Elapsed process time = %llu nanoseconds\n", (long long unsigned int)diff);
printf("Per operation: %lld \n", (long long unsigned int) diff/ (num_threads * num_iterations * 2));
 


  return 0; 
  
}
