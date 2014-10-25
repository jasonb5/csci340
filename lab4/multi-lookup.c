#define DEBUG

#include "multi-lookup.h"

int main(int argc, char **argv) {
  if (argc < 3) {
    error("%s <input files> <output file>", argv[0]);
    
    exit(1);
  }

  int i;
  queue queue;
  void *result;
  sem_t empty, full;
  pthread_mutex_t queue_mut, file_mut;
 
  queue_init(&queue, QUEUE_SIZE); 

  pthread_mutex_init(&queue_mut, NULL);

  pthread_mutex_init(&file_mut, NULL);

  sem_init(&empty, 0, QUEUE_SIZE);

  sem_init(&full, 0, 0);

  int requester_count = argc - 2;
  thread_info *requesters = calloc(requester_count, sizeof(thread_info));

  if (requesters == NULL) {
    error("Failed to allocate memory for requesters");
    
    exit(1);
  }

  int resolver_count = get_resolver_count();
  thread_info *resolvers = calloc(resolver_count, sizeof(thread_info));

  if (resolvers == NULL) {
    error("Failed to allocate memory for resolvers");

    exit(1);
  }

  debug("Spawning %d requester threads", requester_count);

  debug("Spawning %d resolver threads", resolver_count);

  for (i = 0; i < requester_count; ++i) {
    requesters[i].id = i + 1;

    requesters[i].file = argv[(1 + i)];

    requesters[i].queue = &queue;

    requesters[i].empty = &empty;

    requesters[i].full = &full;
  
    requesters[i].queue_mut = &queue_mut;

    pthread_create(&requesters[i].handle, NULL, &requester_thread, &requesters[i]); 
  } 

  for (i = 0; i < resolver_count; ++i) {
    resolvers[i].id = i + 1;

    resolvers[i].file = argv[(argc - 1)];

    resolvers[i].file_mut = &file_mut;

    resolvers[i].queue = &queue;

    resolvers[i].empty = &empty;

    resolvers[i].full = &full;
  
    resolvers[i].queue_mut = &queue_mut;

    pthread_create(&resolvers[i].handle, NULL, &resolver_thread, &resolvers[i]);
  }
 
  for (i = 0; i < requester_count; ++i) {
    pthread_join(requesters[i].handle, &result);
  }

  for (i = 0; i < resolver_count; ++i) {
    sem_wait(&empty);

    pthread_mutex_lock(&queue_mut);

    queue_push(&queue, NULL);

    pthread_mutex_unlock(&queue_mut);

    sem_post(&full);
  }

  for (i = 0; i < resolver_count; ++i) {
    pthread_join(resolvers[i].handle, &result);
  }

  queue_cleanup(&queue);

  free(resolvers);

  free(requesters);

  return 0;
}

void *requester_thread(void *arg) {
  FILE *fp; 
  char *buffer;
  thread_info *thread = arg;

  debug("Requester thread %d processing %s", thread->id, thread->file);

  if ((fp = fopen(thread->file, "r")) == NULL) {
    error("Failed to open %s", thread->file);

    return NULL;
  }

  while (1) {
    buffer = malloc(MAX_NAME_LENGTH * sizeof(char));

    if (fscanf(fp, "%s", buffer) <= 0) {
      break;
    }

    sem_wait(thread->empty);

    pthread_mutex_lock(thread->queue_mut);

    debug("Requester thread %d pushing %s", thread->id, buffer);

    queue_push(thread->queue, buffer);

    pthread_mutex_unlock(thread->queue_mut);

    sem_post(thread->full); 
  }

  free(buffer);

  fclose(fp);

  return NULL;
}

void *resolver_thread(void *arg) {
  FILE *fp;
  char *buffer;
  thread_info *thread = arg;

  if ((fp = fopen(thread->file, "a")) == NULL) {
    error("Failed to open %s", thread->file); 

    return NULL;
  }

  while (1) {
    sem_wait(thread->full);

    pthread_mutex_lock(thread->queue_mut);

    buffer = queue_pop(thread->queue);

    if (buffer == NULL) {
      pthread_mutex_unlock(thread->queue_mut);

      sem_post(thread->empty);

      debug("Resolver thread %d notify of exit", thread->id); 

      break;
    }

    debug("Resolver thread %d popped %s", thread->id, buffer);

    pthread_mutex_unlock(thread->queue_mut);

    sem_post(thread->empty);

   	int i, ret; 
		char *resolved[10];

    ret = dns_lookup(buffer, resolved);

		if (ret == 0) {
      resolved[0] = '\0';
  	
			ret = 1; 
		}
 
    pthread_mutex_lock(thread->file_mut);

		fprintf(fp, "%s", buffer);

		for (i = 0; i < ret; ++i) {
			fprintf(fp, ",%s", resolved[i]);
		}

		fprintf(fp, "\n");

    pthread_mutex_unlock(thread->file_mut);

    free(buffer); 
  } 

  fclose(fp);

  return NULL;
}

int get_resolver_count() {
  int count = MIN_RESOLVER_THREADS;
  int cpu = sysconf(_SC_NPROCESSORS_ONLN);

  if (cpu != -1 && cpu > count) {
    if (MAX_RESOLVER_THREADS != -1 && cpu > MAX_RESOLVER_THREADS) {
      count = MAX_RESOLVER_THREADS;
    } else { 
      count = cpu;
    }
  }

  //return count;
	return 1;
}

int dns_lookup(const char *host, char **result) {
	int i, c;	
	char *temp[25];	
	struct addrinfo hints;
	struct addrinfo *results, *rp;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM; // limit to prevent duplicates
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = 0;

	if (getaddrinfo(host, NULL, &hints, &results) != 0) {
		error("Failed to resolve %s", host);	

		return 0;
	}

	for (i = 0, rp = results; rp != NULL; rp = rp->ai_next) {
		temp[i] = malloc(MAX_IP_LENGTH * sizeof(char));	

		if (rp->ai_family == AF_INET) {	
			struct sockaddr_in *addr_in = (struct sockaddr_in *)rp->ai_addr;

			inet_ntop(rp->ai_family, &addr_in->sin_addr, temp[i], rp->ai_addrlen);			
	
			++i;	
		} else if (IPV6_RESOLVE && rp->ai_family == AF_INET6) {
			struct sockaddr_in6 *addr_in = (struct sockaddr_in6 *)rp->ai_addr;	

			inet_ntop(rp->ai_family, &addr_in->sin6_addr, temp[i], rp->ai_addrlen);	

			++i;
		}
	}

	for (c = 0; c < i; ++c) {
		result[c] = temp[c];
	}

	freeaddrinfo(results);

  return i;
}
