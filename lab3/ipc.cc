// 
// 
// 
// Jason Boutte (JJB87)
//

using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string>
#include <limits.h>
#include <sys/time.h>

#include "helper-routines.h"

/*Define Global Variables*/
pid_t childpid;
timeval t1, t2;
timeval tt1, tt2;
int numtests;
double elapsedTime;
double diffTime;
double tripTime;
double minTime;
double maxTime;
int state;

void sigusr1_handler(int sig) {	
  // Set state to notify parent on full trip
  state = 0;
}

void sigusr2_handler(int sig) {
  // Get time to track trip from child process
  gettimeofday(&tt2, NULL);

  // Calculate child trip time
  diffTime = (tt2.tv_sec - tt1.tv_sec) * 1000.0;
  diffTime += (tt2.tv_usec - tt1.tv_usec) / 1000.0;

  // Adjust trip total
  tripTime += diffTime;

  // Update max trip time
  if (diffTime > maxTime) {
    maxTime = diffTime;
  }

  // Update min trip time
  if (diffTime < minTime) {
    minTime = diffTime;
  }

  // Assign current time to first time
  tt1 = tt2;

  // Finish second leg of trip
  kill(getppid(), SIGUSR1);
}

void sigint_handler(int sig) {
  // Notify child process of end of test
  state = 0;
}

//
// main - The main routine 
//
int main(int argc, char **argv){
	//Initialize Constants here
	
  //variables for Pipe
	int fd1[2],fd2[2], nbytes;	
	char buf; 

  //byte size messages to be passed through pipes	
	char    childmsg[] = "1";
	char    parentmsg[] = "2";
	char    quitmsg[] = "q";
    
  /*Three Signal Handlers You Might Need
   *
   *I'd recommend using one signal to signal parent from child
   *and a different SIGUSR to signal child from parent
   */
  Signal(SIGUSR1,  sigusr1_handler); //User Defined Signal 1
  Signal(SIGUSR2,  sigusr2_handler); //User Defined Signal 2
  Signal(SIGINT, sigint_handler);

  minTime = 1000.0;
  maxTime = 0.0;

  //Default Value of Num Tests
  numtests=10000;
  //Determine the number of messages was passed in from command line arguments
  //Replace default numtests w/ the commandline argument if applicable 
  if(argc<2){
    printf("Not enough arguments\n");

    exit(0);
	}
  
  // If third argument is present we adjust numtests
  if (argc == 3) {
    numtests = atoi(argv[2]);
  }
  
  printf("Number of Tests %d\n", numtests);
  // start timer
	gettimeofday(&t1, NULL); 
	if(strcmp(argv[1],"-p")==0){
		//code for benchmarking pipes over numtests

    // Initialize parent to child pipe
    if (pipe(fd1) == -1) {
      perror("pipe1");

      exit(0);
    }

    // Initialize child to parent pipe
    if (pipe(fd2) == -1) {
      perror("pipe2");
    
      exit(0);
    }

    // Get start time for child process
    gettimeofday(&tt1, NULL);

    childpid = fork();

    if (childpid == -1) {
      perror("fork");

      exit(0);
    }    

    if (childpid == 0) {
      close(fd1[1]);  // Close write end of fd1
      close(fd2[0]);  // Close read end of fd2

      // Wait for parent message. Calculate trip time,
      // time from child write to read
      while ((nbytes = read(fd1[0], &buf, 1)) > 0) {
        gettimeofday(&tt2, NULL);        

        diffTime = (tt2.tv_sec - tt1.tv_sec) * 1000.0;
        diffTime += (tt2.tv_usec - tt1.tv_usec) / 1000.0;

        tripTime += diffTime;

        if (diffTime > maxTime) {
          maxTime = diffTime;
        }

        if (diffTime < minTime) {
          minTime = diffTime;
        } 

        // Break on quit message
        if (buf == 'q') {
          break;
        }

        gettimeofday(&tt1, NULL);

        write(fd2[1], &childmsg, 1);
      }

      close(fd1[0]);
      close(fd2[1]);

      printf("Child's Results for Pipe IPC mechanisms\n");
    } else {
      close(fd1[0]);  // Close read end of fd1
      close(fd2[1]);  // Close write end of fd2

      // Write parent message and wait read child message,
      // calculate trip time
      for (int i = 0; i < numtests; ++i) {
        gettimeofday(&tt1, NULL);

        write(fd1[1], &parentmsg, 1);

        nbytes = read(fd2[0], &buf, 1);

        gettimeofday(&tt2, NULL);

        diffTime = (tt2.tv_sec - tt1.tv_sec) * 1000.0;
        diffTime += (tt2.tv_usec - tt1.tv_usec) / 1000.0;

        tripTime += diffTime;

        if (diffTime > maxTime) {
          maxTime = diffTime;
        }

        if (diffTime < minTime) {
          minTime = diffTime;
        }
      }

      // Notify child of test end
      write(fd1[1], &quitmsg, 1);

      close(fd1[1]);
      close(fd2[0]);

      // Wait for child to exit
      wait(NULL);

      printf("Parent's Results for Pipe IPC mechanisms\n"); 
    }

    printf("Process ID is %d, Group ID is %d\n", getpid(), getgid()); 
    printf("Round trip times\n");
    printf("Average %f\n", tripTime / (double)numtests);
    printf("Maximum %f\n", maxTime);
    printf("Minimum %f\n", minTime);

    // stop timer
		gettimeofday(&t2, NULL);

		// compute and print the elapsed time in millisec
		elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
		elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
		printf("Elapsed Time %f\n", elapsedTime);
	}
	if(strcmp(argv[1],"-s")==0){
		//code for benchmarking signals over numtests

    // Set initial state for child process
    state = 1;

    gettimeofday(&tt1, NULL);
 
    childpid = fork();

    if (childpid == -1) {
      perror("fork");

      exit(1);
    }

    if (childpid == 0) {
      // Sleep until test is over
      while (state) { }

      // Get final time for last leg of child trip
      gettimeofday(&tt2, NULL);

      diffTime = (tt2.tv_sec - tt1.tv_sec) * 1000.0;
      diffTime += (tt2.tv_usec - tt1.tv_usec) / 1000.0;

      tripTime += diffTime;

      if (diffTime > maxTime) {
        maxTime = diffTime;
      }

      if (diffTime < minTime) {
        minTime = diffTime;
      }

      printf("Child's Results for Signal IPC mechanisms\n");
    } else {
      int status;

      // Call SIGUSR2 of child process to start trip,
      // finish trip on SIGUSR1 of parent process,
      // which changes state to false
      for (int i = 0; i < numtests; ++i) {
        state = 1;

        gettimeofday(&tt1, NULL);

        kill(childpid, SIGUSR2);

        while (state) { }     
 
        gettimeofday(&tt2, NULL);

        diffTime = (tt2.tv_sec - tt1.tv_sec) * 1000.0;
        diffTime += (tt2.tv_usec - tt1.tv_usec) / 1000.0;

        tripTime += diffTime;

        if (diffTime > maxTime) {
          maxTime = diffTime;
        }

        if (diffTime < minTime) {
          minTime = diffTime;
        }
      }

      // Notify child of test end
      kill(childpid, SIGINT);

      // Wait for child to exit
      wait(NULL);

      printf("Parent's Results for Signal IPC mechanisms\n");
    }

    printf("Process ID is %d, Group ID is %d\n", getpid(), getgid()); 
    printf("Round trip times\n");
    printf("Average %f\n", tripTime / (double)numtests);
    printf("Maximum %f\n", maxTime);
    printf("Minimum %f\n", minTime);
 
		// stop timer
		gettimeofday(&t2, NULL);

		// compute and print the elapsed time in millisec
		elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
		elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
		printf("Elapsed Time %f\n", elapsedTime);
	}
}
  










