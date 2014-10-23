#define DEBUG

#include "multi-lookup.h"

int main(int argc, char **argv) {
  if (argc < 3) {
    error("%s <input files> <output file>", argv[0]);
    
    exit(1);
  }

  queue queue;
  void *result;
  FILE *out_file; 
  pthread_mutex_t file_mutex; 
  pthread_mutex_t queue_mutex;
  thread_info *requesters = calloc(1, sizeof(thread_info));
  thread_info *resolvers = calloc(1, sizeof(thread_info));

  if (requesters == NULL) {
    error("failed to allocate memory");
  
    exit(1);
  }

  if (resolvers == NULL) {
    error("failed to allocate memory");

    exit(1);
  }

  if ((out_file = fopen(argv[2], "w+")) == NULL) {
    error("failed to open output file");

    exit(1);
  }

  queue_init(&queue, 10);

  pthread_mutex_init(&queue_mutex, NULL);

  pthread_mutex_init(&file_mutex, NULL);

  requesters[0].id = 0;

  requesters[0].file = argv[1];

  requesters[0].queue = &queue;

  requesters[0].queue_mutex = &queue_mutex;

  pthread_create(&requesters[0].thread, NULL, &requester_thread, &requesters[0]);

  resolvers[0].id = 0;

  resolvers[0].queue = &queue;

  resolvers[0].out_file = out_file;

  resolvers[0].file_mutex = &file_mutex;

  resolvers[0].queue_mutex = &queue_mutex;

  pthread_create(&resolvers[0].thread, NULL, &resolver_thread, &resolvers[0]);

  pthread_join(requesters[0].thread, &result);

  pthread_join(resolvers[0].thread, &result); 

  pthread_mutex_destroy(&file_mutex);
  
  pthread_mutex_destroy(&queue_mutex);

  queue_cleanup(&queue);

  fclose(out_file);

  free(resolvers);

  free(requesters);

  return 0;
}

void *requester_thread(void *arg) {
  FILE *fp;
  char *buffer; 
  thread_info *thread = arg;
  queue *queue = thread->queue;
  pthread_mutex_t *mutex = thread->queue_mutex;

  debug("Requester thread %d processing %s", thread->id, thread->file);

  if ((fp = fopen(thread->file, "r")) == NULL) {
    error("failed to open file %s", thread->file);

    return NULL;
  }

  while (1) {
    buffer = malloc(1024 * sizeof(char));
   
     if (fscanf(fp, "%1024s", buffer) < 0) {
      free(buffer);
      
      break;
    }
  
    while (queue_is_full(queue)) {
      usleep(100);
    }

    pthread_mutex_lock(mutex);

    debug("Requester thread %d pushing %s", thread->id, buffer);

    queue_push(queue, buffer);

    buffer = NULL;

    pthread_mutex_unlock(mutex); 
  }

  fclose(fp);

  return NULL;
}

void *resolver_thread(void *arg) {
  char *buffer; 
  char resolved[1024]; 
  thread_info *thread = arg;
  queue *queue = thread->queue; 
  
  while (queue_is_empty(queue)) {
    usleep(100);
  }

  while (1) {
    pthread_mutex_lock(thread->queue_mutex);

    buffer = queue_pop(queue);

    pthread_mutex_unlock(thread->queue_mutex);

    if (buffer == NULL) {
      break;
    }

    debug("Resolver thread %d popping %s", thread->id, buffer);

    if (dnslookup(buffer, resolved, 1024) == UTIL_FAILURE) {
      resolved[0] = '\0';
    }

    debug("Resolver thread %d %s returned as %s", thread->id, buffer, resolved);

    fprintf(thread->out_file, "%s,%s\n", buffer, resolved);

    free(buffer);
  }

  return NULL;
}
