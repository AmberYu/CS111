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
static int** string_array;
static SortedListElement_t* head = NULL;

//demonstrate conflicts between 
// 1) inserts
// 2) deletes
// 3) inserts and lookups
// 4) deletes and loopups
#include "SortedList.h"

void SortedList_insert(SortedList_t *list, SortedListElement_t *element){   //element is current ndoe
  if (list==NULL)
  {
    list = element;
    list->next = element;
    list->prev = element;
    return;
  }
  SortedListElement_t *p = list;
  SortedListElement_t *n = list->next;
  while(n!=list){
    //list is not empty
    if(strcmp(element->key,n->key)<=0)
      break;
    p = n;
    n = n->next;

  }
  //we may have conflict since the list structure could change before we insert the element
  //if we call thread_yield here, we can see the incorrect value in a few run
  //how do we prevent this conflict
  //method1. lock the whole thing, lock, insert, unlock
  element->prev = p;
  element->next = n;
  p->next = element;
  n->prev = element;
}

int SortedList_delete( SortedListElement_t *element){
  //if you want to delete a node, you have to call lookup first 
  //and then the return value of lookup will the input of delete
  //lock
  //lookup   lookup and delete have to be in the same critical section, otherwise the returned value could be changed 
  //delete   before passing to delete function
  //unlock

  SortedListElement_t *n = element->next;
  SortedListElement_t *p = element->prev;
  if(n->prev!=element){
    return -1;
  }
  if(p->next!=element){
    return -1;
  }
  //conflits can happen before you delete the node, someone could have changed the structure of the list
  n->prev = p;
  p->next = n;
  element->next=NULL;
  element->prev = NULL;
  return 0;
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key){
  SortedListElement_t *cur = list;
  while(cur){
    if (strcomp(cur->key,key)==0)
      break;
    cur = cur->next;
  }
  return cur;
}

int SortedList_length(SortedList_t *list){
  SortedListElement_t *cur = list;
  int length=0;
  while(cur){
    length++;
    cur = cur->next;
  }
  return length;
}
//Helper function
//Generate a random string
char *rand_string(int length) {
    char *string = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,.-#'?!";
    size_t stringLen = 26*2+10+7;
    char *randomString;
    
    randomString = malloc(sizeof(char) * (length +1));
    
    if (!randomString) {
        return (char*)0;
    }
    
    unsigned int key = 0;
    
    for (int n = 0;n < length;n++) {
        key = rand() % stringLen;
        randomString[n] = string[key];
    }
    
    randomString[length] = '\0';
    
    return randomString;
}

//Generate a random number, we assume the number is within 1~20
int rand_num(){
  srand(time(NULL));
  return rand();
}

//Initialize string array
void generate_string_array(){
  string_array = new int*[num_threads];   //number of rows is the number of threads
  for(int i = 0; i < num_threads; ++i)
    string_array[i] = new int[num_iterations];
  //generate random string and store into the array
  for (int i = 0; i < num_threads; ++i)
  {
    for (int j = 0; j < num_iterations; ++j)
    {
      int len = rand_num()%20+1;
      string_array[i][j] = rand_string(len);
    }
  }
}
//first implement the three function without locking and see how the race condition impact the result
//then add the locking to critical section

void thread_func(void *arg){

  //for each thread, do some preparation work
  int i;
  for(i = 0; i < num_iterations; ++i){
    //insert
    SortedListElement_t* node;
    node->key = string_array[*arg][i];
    int ret = SortedList_insert(head, node);
    if (ret)
      fprintf(stderr,"Failed with error [%s]\n",strerror(errno)); 
  }
  //lookup a string in the string array in order, return the node needs to be deleted

  for(i = 0; i < num_iterations; ++i){
    //delete
    int cur_len = SortedList_length(head);
    int idx = rand_num()%cur_len;
    int ret = SortedList_delete(SortedList_lookup(head, string_array[arg][idx]));
    if(ret)
      fprintf(stderr,"Failed with error [%s]\n",strerror(errno)); 
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

  //initialize string array
  generate_string_array();

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
    int ret = pthread_create(&threads[i], NULL,(void *) thread_func,(void *) i);
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