#include "multi-lookup.h"

int main(int argc, char **argv) {
 	// Check for minimum required arguments 
	if (argc < 3) {
    error("%s <input files> <output file>", argv[0]);
    
    exit(1);
  }

  int i;
 	FILE *fp; 
	queue queue;
  sem_t empty, full;
  pthread_mutex_t queue_mut = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t file_mut = PTHREAD_MUTEX_INITIALIZER;

	// Initializes queue
  if (queue_init(&queue, QUEUE_SIZE) == QUEUE_FAILURE) {
		error("Failed to initialize queue");

		exit(1);
	} 
 
	// Initializes empty semaphore, allowing QUEUE_SIZE
	// requester threads to enter critical section 
  if (sem_init(&empty, 0, QUEUE_SIZE) == -1) {
		error("Failed to initialize empty semaphore");
		
		exit(1);
	}

	// Initializes full semaphore, allowing QUEUE_SIZE
	// resolver threads to enter critical section
  if (sem_init(&full, 0, 0) == -1) {
		error("Failed to initialize full semaphore");
	
		exit(1);
	}

	// Initialize requester resources
  int requester_count = argc - 2;
  thread_info *requesters = calloc(requester_count, sizeof(thread_info));

  if (requesters == NULL) {
    error("Failed to allocate memory for requesters");
    
    exit(1);
  }

	// Initializes resolver resources
  int resolver_count = get_resolver_count();
  thread_info *resolvers = calloc(resolver_count, sizeof(thread_info));

  if (resolvers == NULL) {
    error("Failed to allocate memory for resolvers");

		free(requesters);

    exit(1);
  }

	// Opens output file for writing
	if ((fp = fopen(argv[(argc - 1)], "w")) == NULL) {
		error("Failed to open output file");

		free(requesters);
		free(resolvers);

		exit(1);
	}

  debug("Spawning %d requester threads", requester_count);

  debug("Spawning %d resolver threads", resolver_count);
	
	// Create single thread for each input file
  for (i = 0; i < requester_count; ++i) {
    requesters[i].id = i + 1;

    requesters[i].file = argv[(1 + i)];

    requesters[i].queue = &queue;

    requesters[i].empty = &empty;

    requesters[i].full = &full;
  
    requesters[i].queue_mut = &queue_mut;

    pthread_create(&requesters[i].handle, NULL, &requester_thread, &requesters[i]); 
  } 

	// Create resolver threads matching cpu count
  for (i = 0; i < resolver_count; ++i) {
    resolvers[i].id = i + 1;

    resolvers[i].out_fp = fp;

    resolvers[i].file_mut = &file_mut;

    resolvers[i].queue = &queue;

    resolvers[i].empty = &empty;

    resolvers[i].full = &full;
  
    resolvers[i].queue_mut = &queue_mut;

    pthread_create(&resolvers[i].handle, NULL, &resolver_thread, &resolvers[i]);
  }

	// Block until requesters are done, guarantees all
	// possible input values are in the queue 
  for (i = 0; i < requester_count; ++i) {
    pthread_join(requesters[i].handle, NULL);
  }

	// Queue one end value for each resolver thread
  for (i = 0; i < resolver_count; ++i) {
    sem_wait(&empty);

    pthread_mutex_lock(&queue_mut);

    queue_push(&queue, NULL);

    pthread_mutex_unlock(&queue_mut);

    sem_post(&full);
  }

	// Block until all resolvers are done
  for (i = 0; i < resolver_count; ++i) {
    pthread_join(resolvers[i].handle, NULL);
  }

	// Clean up resources
	fclose(fp);

	pthread_mutex_destroy(&file_mut);

	pthread_mutex_destroy(&queue_mut);

  queue_cleanup(&queue);

  free(resolvers);

  free(requesters);

  return 0;
}

// Opens input file, reads each value and pushes
// into shared queue
void *requester_thread(void *arg) {
  FILE *fp; 
  char *buffer;
  thread_info *thread = arg;

  debug("Requester thread %d processing %s", thread->id, thread->file);

  if ((fp = fopen(thread->file, "r")) == NULL) {
    error("Failed to open %s", thread->file);

    return NULL;
  }

	// Repeat for each line of input file
  while (1) {
		// Allocate buffer for hostname
    buffer = malloc(MAX_NAME_LENGTH * sizeof(char));

    if (fscanf(fp, "%s", buffer) <= 0) {
      break;
    }

		// Wait to enter critical section,
		// guarantees space in queue
    sem_wait(thread->empty);

		// Ensure mutual exclusion
    pthread_mutex_lock(thread->queue_mut);

    debug("Requester thread %d pushing %s", thread->id, buffer);
	
		// Push host into queue
    queue_push(thread->queue, buffer);

		// Release mutex
    pthread_mutex_unlock(thread->queue_mut);

    sem_post(thread->full); 
  }

	// Clean resources
  free(buffer);

  fclose(fp);

  return NULL;
}

// Processes each hostname and resolves to ip
// writes all ip values to results file
void *resolver_thread(void *arg) {
  char *buffer;
  thread_info *thread = arg;

  while (1) {
		// Wait to enter critical section
		// Ensures something to process 
    sem_wait(thread->full);

		// Ensure mutual exclusion
    pthread_mutex_lock(thread->queue_mut);

		// Pop value from queue
    buffer = queue_pop(thread->queue);

		// Check for end value
    if (buffer == NULL) {
			pthread_mutex_unlock(thread->queue_mut);

      sem_post(thread->empty);

      debug("Resolver thread %d notify of exit", thread->id); 

      break;
    }

    debug("Resolver thread %d popped %s", thread->id, buffer);

    pthread_mutex_unlock(thread->queue_mut);

		// Increment empty semaphore freeing
		// spaces for another host
    sem_post(thread->empty);

  	int i, ret; 
		char **resolved;

		// Allocate space for 20 ip's
		resolved = malloc(20 * sizeof(char**));

		// Resolves hostname
  	ret = dns_lookup(buffer, resolved);

		// Ensure mutual exclusion for output file
    pthread_mutex_lock(thread->file_mut);

		// Write hostname
		fprintf(thread->out_fp, "%s", buffer);

		// Write empty value if hostname fails to resolve
		// or write all returned ip's
		if (ret == 0) {
			fprintf(thread->out_fp, ",");
		} else {
			for (i = 0; i < ret; ++i) {
				fprintf(thread->out_fp, ",%s", resolved[i]);
				
				// Free resources	
				free(resolved[i]);
			}
		}

		fprintf(thread->out_fp, "\n");

    pthread_mutex_unlock(thread->file_mut);

		// Free resources
		free(resolved);

    free(buffer); 
  } 

  return NULL;
}

// Returns optimal resolver threads
// Applies min and max limits to resolver threads
int get_resolver_count() {
  int count = MIN_RESOLVER_THREADS;
  int cpu = sysconf(_SC_NPROCESSORS_ONLN); // Queries os for cpu count

  if (cpu != -1 && cpu > count) {
    if (MAX_RESOLVER_THREADS != -1 && cpu > MAX_RESOLVER_THREADS) {
      count = MAX_RESOLVER_THREADS;
    } else { 
      count = cpu;
    }
  }

	return count;
}

// Returns number of ip's resolved from hostname
// Fills result with each ip and returns the count
int dns_lookup(const char *host, char **result) {
	int i = 0;
	struct addrinfo hints;
	struct addrinfo *results, *rp;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM; // limit to prevent duplicates
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = 0;

	// Resolves host to ip's
	if (getaddrinfo(host, NULL, &hints, &results) != 0) {
		error("Failed to resolve %s", host);	

		return 0;
	}

	// Iterates through results
	for (i = 0, rp = results; rp != NULL; rp = rp->ai_next) {
		if (rp->ai_family == AF_INET) {	
			result[i] = malloc(MAX_IP_LENGTH * sizeof(char));	

			struct sockaddr_in *addr_in = (struct sockaddr_in *)rp->ai_addr;

			inet_ntop(rp->ai_family, &addr_in->sin_addr, result[i], MAX_IP_LENGTH);			
	
			++i;	
		} else if (IPV6_RESOLVE && rp->ai_family == AF_INET6) {
			result[i] = malloc(MAX_IP_LENGTH * sizeof(char));	
			
			struct sockaddr_in6 *addr_in = (struct sockaddr_in6 *)rp->ai_addr;	

			inet_ntop(rp->ai_family, &addr_in->sin6_addr, result[i], rp->ai_addrlen);	

			++i;
		}
	}

	freeaddrinfo(results);

  return i;
}
