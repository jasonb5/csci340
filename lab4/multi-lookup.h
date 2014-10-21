#ifndef LAB4_MULTI_LOOKUP_H_
#define LAB4_MULTI_LOOKUP_H_

#include <stdio.h>

#ifndef DEBUG
#define debug(M, ...)
#else
#define debug(M, ...) fprintf(stderr, "%s:%d: " M "\n", \
  __FILE__, __LINE__, ##__VA_ARGS__)
#endif // DEBUG

#define error(M, ...) fprintf(stderr, M "\n", ##__VA_ARGS__)

#include <stdlib.h>
#include <pthread.h>

typedef struct thread_info_s {
  char *file;
  pthread_t id;
  pthread_mutex_t *mutex;
} thread_info;

#define MAX_LINE 1025

int get_requester_threads(void);
void *requester_thread(void *arg);

#endif // LAB4_MULTI_LOOKUP_H_
