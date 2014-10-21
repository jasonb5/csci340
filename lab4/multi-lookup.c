#define DEBUG

#include "multi-lookup.h"

int main(int argc, char **argv) {
  if (argc < 3) {
    error("%s <input files> <output file>", argv[0]);    
  
    exit(1);
  }

  int i;
  void *result;
  thread_info *requesters;
  int threads = get_requester_threads();

  requesters = calloc(threads, sizeof(thread_info));

  if (requesters == NULL) {
    error("memory allocation");
  
    exit(1);
  }

  for (i = 0; i < threads; ++i) {
    requesters[i].file = argv[1+i];

    pthread_create(&requesters[i].id, NULL, &requester_thread, &requesters[i]);
  }

  for (i = 0; i < threads; ++i) {
    pthread_join(requesters[i].id, &result);
  }

  return 0;
}

int get_requester_threads(void) {
  return 1;
}

void *requester_thread(void *arg) {
  FILE *fp; 
  char *buffer;
  thread_info *thread_info = arg;

  debug("opening %s", thread_info->file);

  if ((fp = fopen(thread_info->file, "r")) == NULL) {
    error("failed to open %s", thread_info->file);

    return NULL;
  }

  while (1) {
    buffer = malloc(MAX_LINE * sizeof(char));

    if (fscanf(fp, "%1024s", buffer) > 0) {
      free(buffer);

      break;
    }
  }

  fclose(fp);

  return NULL;
}
