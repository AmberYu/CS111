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
#include "SortedList.h"

#define no_argument 0
#define required_argument 1 
#define optional_argument 2

#define ITER     'a'
#define THREADS  'b'
#define YIELD    'c'
#define SYNC     'd'


static  int num_iterations;
static  int num_threads;

int opt_yield=0;
char opt_sync='n';

static pthread_mutex_t lock;
static volatile int spin_lock=0;

static char*** string_array;
SortedListElement_t* head;
int list_length=0;
static int* delete_sequence;

void init(){
	head = (SortedListElement_t*) malloc(sizeof(SortedListElement_t));
	head->key = NULL;
	head->next=head;
	head->prev=head;
}


void SortedList_insert(SortedList_t *list, SortedListElement_t *element){   //element is current ndoe

  printf("inserting %s\n",element->key);
  SortedListElement_t *p = list;
  SortedListElement_t *n = list->next;
  while(n!=list){
    //list is not empty
    if(strcmp(element->key,n->key)<=0)
      break;
    p = n;
    n = n->next;

  }
  // if(opt_yield & INSERT_YIELD){
	 //  pthread_yield();
  // }
  element->prev = p;
  element->next = n;
  p->next = element;
  n->prev = element;
  list_length++;
  
}
/* int SortedList_delete( SortedListElement_t *element)
  If you want to delete a node, you have to call lookup first to find which node to delete
  
  //lock
  //lookup   lookup and delete have to be in the same critical section, otherwise the returned value could be changed 
  //delete   before passing to delete function
  //unlock
*/
int SortedList_delete( SortedListElement_t *element){


	  
  SortedListElement_t *n = element->next;
  SortedListElement_t *p = element->prev;
  if(n->prev!=element){
    return -1;
  }
  if(p->next!=element){
    return -1;
  }
  //conflits can happen before you delete the node, someone could have changed the structure of the list

  // if(opt_yield & DELETE_YIELD){
	 //  pthread_yield();
  // }
  
  n->prev = p;
  p->next = n;
  element->next=NULL;
  element->prev = NULL;
  list_length--;
  
  return 0;
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key){

  SortedListElement_t *cur = NULL;
  if(list->next)
    cur = list->next;
  // if(opt_yield & SEARCH_YIELD){
	 //  pthread_yield();
  // }
 
  while(cur!=list){
    if (strcmp(cur->key,key)==0)
      break;
    cur = cur->next;
  }

  return cur;
}

int SortedList_length(/*SortedList_t *list*/){  
  // if(opt_yield & SEARCH_YIELD){
	 //  pthread_yield();
  // }

  return list_length;
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
    int n;
    for (n = 0;n < length;n++) {
        key = rand() % stringLen;
        randomString[n] = string[key];
    }
    
    randomString[length] = '\0';
    
    return randomString;
}

//Generate a random number, we assume the number is within 1~20
int rand_num(){
  return rand();
}

//generate_delete_sequence
void generate_delete_sequence(){
	delete_sequence = (int*) malloc(sizeof(int)*num_iterations);
	int visited[num_iterations];
	memset(visited,0,sizeof(visited));
	int i,lottery;
	for(i=0;i<num_iterations;i++){
		do{
			lottery = rand_num()%num_iterations;
		}while(visited[lottery]);
		visited[lottery]=1;
		delete_sequence[i] = lottery;
	}
}

//Initialize string array
void generate_string_array(){
  string_array = malloc(sizeof(char**)*num_threads);  //number of rows is the number of threads
  int i,j;
  for(i = 0; i < num_threads; ++i)
    string_array[i] = malloc(sizeof(char*)*num_iterations);
  //generate random string and store into the array

  for (i = 0; i < num_threads; ++i)
  {
	  //printf("%d num_threads", i);
    for (j = 0; j < num_iterations; ++j)
    {
      int len = rand_num()%10+1;
      string_array[i][j] = rand_string(len);
	  //printf("%s  ",string_array[i][j]);
    }
  }
}

void thread_func(void* arg){

  int thread_id = *((int*) arg);
  
  int i;
  //insert
  for(i = 0; i < num_iterations; ++i){
    SortedListElement_t* node;
    node = (SortedListElement_t*)malloc(sizeof(SortedListElement_t));
    node->key = string_array[thread_id][i];
    node->prev=node;   
    node->next=node;
    //mutex
    if(opt_sync=='m'){
      pthread_mutex_lock(&lock);      
      SortedList_insert(head, node);
      pthread_mutex_unlock(&lock);
    }
    //spin lock
    else if(opt_sync=='s'){      
      while(__sync_lock_test_and_set(&spin_lock,1));
      SortedList_insert(head, node);
      __sync_lock_release(&spin_lock);
    }
    //no locking
    else SortedList_insert(head, node);
  }
  //delete
  for(i = 0; i < num_iterations; ++i){
    if(opt_sync=='m')
      pthread_mutex_lock(&lock);
    if(opt_sync=='s')
      while(__sync_lock_test_and_set(&spin_lock,1));
    char* key_to_delete = string_array[thread_id][delete_sequence[i]];
     //printf("key to delete is %s\n",key_to_delete);
    
    SortedListElement_t* node_to_delete = SortedList_lookup(head, key_to_delete);
    //printf("actual key to delete is %s\n",node_to_delete->key);
    int ret = SortedList_delete(node_to_delete);
    if(ret<0)
      fprintf(stderr,"Failed with error [%s]\n",strerror(errno)); 
    if(opt_sync=='m')
      pthread_mutex_unlock(&lock);
    if(opt_sync=='s')
      __sync_lock_release(&spin_lock);    
  }
}

// void print_array(){
// 	int i,j;
// 	for(i = 0; i < num_threads; ++i){
// 		// printf("%d Threads:\n", i); 
// 		for(j = 0; j < num_iterations; ++j)
// 		{
// 			printf("%s  ",string_array[i][j]);
// 		}
// 		printf("\n");
// 	}
// }

int main(int argc, char * argv[])
{
  const struct option longopts[] =
  {
    {"iter",  optional_argument, 0, ITER },
    {"threads",  optional_argument, 0, THREADS },
    {"yield",  optional_argument, 0, YIELD },
	  {"sync",  optional_argument, 0, SYNC },
    {0,           0,               0,  0   }  //denote end
  };

  //define time usage
  int iarg = 0;
  int index;
  errno = 0;

  pthread_mutex_init(&lock,NULL);
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
			  if(strcmp(optarg,"i")==0)
				  opt_yield = INSERT_YIELD;
			  else if(strcmp(optarg,"d")==0)
				  opt_yield = DELETE_YIELD;
			  else if(strcmp(optarg,"is")==0)
				  opt_yield = INSERT_YIELD | SEARCH_YIELD;
			  else if(strcmp(optarg,"ds")==0)
				  opt_yield = DELETE_YIELD | SEARCH_YIELD;
		  }
		  else opt_yield=0;
        break;
      }
	  
	  case SYNC:
	  {
		  if(optarg){
			  if(strcmp(optarg,"m")==0)
				  opt_sync = 'm';
			  else if(strcmp(optarg,"s")==0)
				  opt_sync = 's';
		  }
		  break;
	  }
    }
  }
  
  //initialize string array
  srand(time(NULL));
  generate_string_array();
  //print_array();
  //initialize list
  init();
  //generate delete sequence
  generate_delete_sequence();
  
  struct timespec start,end;
  uint64_t diff;
  //clock_gettime(CLOCK_MONOTONIC, &start);
  pthread_t *threads;
  threads = malloc(sizeof(pthread_t)*num_threads);
  int i;
  for( i = 0; i < num_threads; ++i){
	  printf("thread_id: %d\n",i);
    int ret = pthread_create(&threads[i], NULL,(void *) thread_func,(void*) &i);
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

  //clock_gettime(CLOCK_MONOTONIC, &end);
  //diff = 1000000000 * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;

  //int length = SortedList_length(head);
free(string_array);
printf("%d threads * %d iterations * (add + substract) =  %d operations\n", num_threads, num_iterations, (num_threads * num_iterations * 2));
printf("List length = %d\n", list_length);
//printf("Elapsed process time = %llu nanoseconds\n", (long long unsigned int)diff);
//printf("Per operation: %lld \n", (long long unsigned int) diff/ (num_threads * num_iterations * 2));
 
  return 0; 
  
}
