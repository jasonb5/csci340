#ifndef LAB4_MULTI_LOOKUP_H_
#define LAB4_MULTI_LOOKUP_H_

#include <stdio.h>

#ifndef DEBUG
#define debug(M, ...)
#else
#define debug(M, ...) fprintf(stdout, "DEBUG:%s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__);
#endif

#define error(M, ...) fprintf(stderr, "Error: " M "\n", ##__VA_ARGS__);

#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "util.h"
#include "queue.h"

typedef struct thread_info_s {
  int id; 
  char *file;
  queue *queue;
  FILE *out_file; 
  pthread_t thread;
  pthread_mutex_t *file_mutex;
  pthread_mutex_t *queue_mutex;
} thread_info;

void *requester_thread(void *arg);
void *resolver_thread(void *arg);

#endif // LAB4_MULTI_LOOKUP_H_
