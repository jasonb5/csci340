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

#define QUEUE_SIZE 10
#define MAX_LINE "1024"

#define MAX_INPUT_FILES 10
#define MAX_RESOLVER_THREADS 10
#define MIN_RESOLVER_THREADS 2
#define MAX_NAME_LENGTH 1025
#define MAX_IP_LENGTH INET6_ADDRSTRLEN 

typedef struct thread_info_s {
  int id;
  char *file;
  queue *queue; 
  pthread_t handle;
  sem_t *empty, *full;
  pthread_mutex_t *file_mut;
  pthread_mutex_t *queue_mut;
} thread_info;

void *requester_thread(void *arg);
void *resolver_thread(void *arg);

int get_resolver_count();

int dns_lookup(const char *host, char *results, int size);

#endif // LAB4_MULTI_LOOKUP_H_
