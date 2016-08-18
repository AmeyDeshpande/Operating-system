#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#include "util.h"
#include "queue.h"
#include "multi-lookup.h"

#define USAGE "<fnamePath> <outputFilePath>"
#define MAX_NAME_LENGTH 1025
#define INPUTFS "%1024s"
#define MAX_RESOLVER_THREAD sysconf(_SC_NPROCESSORS_ONLN)  //TO IDENTIFY THE NUMBER OF CPU CORES

queue q;
pthread_mutex_t queueLock;
pthread_mutex_t fileLock;
pthread_cond_t eBrake;
FILE* result = NULL;
int runningThreads = 0;

void* Requester(void* inf)
{
  char hostname[MAX_NAME_LENGTH];
  char* host;
  FILE* fname = fopen(inf, "r");
  
  if(!fname)
  {
    printf("Cannot open file");
    return NULL;
  }
  while(fscanf(fname, INPUTFS, hostname) > 0)
  {
    host = malloc(MAX_NAME_LENGTH);
    strncpy(host, hostname, MAX_NAME_LENGTH); 
    while(queue_is_full(&q)) 
      usleep( rand() % 100 );
    pthread_mutex_lock(&queueLock);
    queue_push(&q, host);
   	pthread_mutex_unlock(&queueLock);  
  }
  fclose(fname);
  return NULL;
}

void* Resolver()
{
  char* hostname;
  char ip[INET6_ADDRSTRLEN];
  while(!queue_is_empty(&q) || !runningThreads)
  {
    pthread_mutex_lock(&queueLock);
    if(!queue_is_empty(&q))
	  {
	    hostname = queue_pop(&q);	  
	    if(hostname != NULL)
	    {
	      pthread_mutex_unlock(&queueLock);	      
	      if(dnslookup(hostname, ip, sizeof(ip)) == UTIL_FAILURE)
	      {
			      fprintf(stderr, "dnslookup error: %s\n", hostname);
			      strncpy(ip, "", sizeof(ip));
	      }
	      pthread_mutex_lock(&fileLock);
	      fprintf(result, "%s,%s\n", hostname, ip);
	      pthread_mutex_unlock(&fileLock);
	    }
	    free(hostname);
	  }
    else
	  {
	    pthread_mutex_unlock(&queueLock);
	  }  
  }

  return NULL;
}

int main(int argc, char* argv[])
{
  int numFiles = argc - 2;
  int i;
  pthread_mutex_init(&queueLock, NULL);
  pthread_mutex_init(&fileLock, NULL);
  
  queue_init(&q, 50);
  
  pthread_t requestThreads[argc-1];
  pthread_t resolverThreads[MAX_RESOLVER_THREAD];
  
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  
  if(argc < 3)
  {
    fprintf(stderr, "Not enough arguments: %d\n", (argc - 1));
    fprintf(stderr, "Usage:\n %s %s\n", argv[0], USAGE);
    return EXIT_FAILURE;
  }
    
  result = fopen(argv[(argc-1)], "w");
  if(!result)
  {
    perror("Error Opening Output File");
    return EXIT_FAILURE;
  }
  
  for( i=1; i<(argc-1); i++)
  {
    if(pthread_create(&requestThreads[i-1], &attr, Requester, argv[i]))
    {
      printf("Error in creating thread\n");
    }
  }

  for( i = 0; i < MAX_RESOLVER_THREAD; ++i)
  {
    if(pthread_create(&resolverThreads[i], &attr, Resolver, NULL))
   	{
      printf("Error in creating thread\n");
   	}
  }
  
  for( i = 0; i < numFiles; ++i)
  {      
    if(pthread_join( requestThreads[i],  NULL))
   	{
      printf("Thread error occured\n");
   	}
  }
  runningThreads = 1;

  for( i = 0; i < MAX_RESOLVER_THREAD; ++i)
  {
    if(pthread_join( resolverThreads[i], NULL))
   	{
	    printf("Thread error occured\n");
   	}    
  }
  fclose(result);
  queue_cleanup(&q);
  pthread_mutex_destroy(&queueLock);
  pthread_mutex_destroy(&fileLock);
  
  return EXIT_SUCCESS;
}