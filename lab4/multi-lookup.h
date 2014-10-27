#ifndef LAB4_MULTI_LOOKUP_H_
#define LAB4_MULTI_LOOKUP_H_

#include <stdio.h>

#ifndef DEBUG
#define debug(M, ...)
#else
#define debug(M, ...) fprintf(stdout, "%s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#define error(M, ...) fprintf(stderr, "ERROR:%s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <semaphore.h>

#include "util.h"
#include "queue.h"

// Enables debug printing
#define DEBUG

// Enables experimental IPV6 support
//#define IPV6_SUPPORT

#ifdef IPV6_SUPPORT
#define IPV6_RESOLVE 1
#else
#define IPV6_RESOLVE 0
#endif

// Sets max queue size
#define QUEUE_SIZE 50
// Limits maximum resolver threads
// value of -1 set no max
#define MAX_RESOLVER_THREADS 10
// Limits minimum resolver threads
#define MIN_RESOLVER_THREADS 2
// Limits maximum hostname
#define MAX_NAME_LENGTH 1025
// Limits maximum IP length
#define MAX_IP_LENGTH INET6_ADDRSTRLEN 

// Structure used to pass data to threads
typedef struct thread_info_s {
  int id;
	FILE *out_fp;
  char *file;
  queue *queue; 
  pthread_t handle;
  sem_t *empty, *full;
  pthread_mutex_t *file_mut;
  pthread_mutex_t *queue_mut;
} thread_info;

// Processes input files, queues each 
// hostname into queue
void *requester_thread(void *arg);

// Processes each hostname and writes
// all ip's to output file
void *resolver_thread(void *arg);

// Determines number of resolver threads
int get_resolver_count();

// Resolves hostname to ip's
int dns_lookup(const char *host, char **results);

#endif // LAB4_MULTI_LOOKUP_H_
