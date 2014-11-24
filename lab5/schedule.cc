#include "schedule.h"
#include <stdlib.h>

struct Node {
	int pid;
	int quanta;
	Node *next;
	Node *prev;
};

struct Queue {
	Node *curr;
	Node *head;
	Node *tail;
};

const int kLevels = 4;

int current;
struct Queue levels[kLevels];
int prior_translation[] = { 4, 3, 2, 1 };

#include <stdio.h>
#define debug(M, ...) fprintf(stdout, M "\n", ##__VA_ARGS__)

int find_node(int pid, struct Node **node, int *level) {
	int x;
	struct Node *n;

	for (x = 0; x < kLevels; ++x) {
		n = levels[x].head;

		while (n) {
			if (n->pid == pid) {
				*node = n;

				*level = x;

				return 0;
			}

			n = n->next;
		}	
	}

	return 1;
}

/*
 * Function to initialize any global variables for the scheduler. 
 */
void init(){
	int x;

	current = 0;

	for (x = 0; x < kLevels; ++x) {
		levels[x].curr = levels[x].head = levels[x].tail = NULL;
	}
}

/*
 * Function to add a process to the scheduler
 * @Param pid - the ID for the process/thread to be added to the 
 *      scheduler queue
 * @Param priority - priority of the process being added
 * @return true/false response for if the addition was successful
 */
int addProcess(int pid, int priority){
	int p = priority - 1;
	struct Node *n = (struct Node *)calloc(1, sizeof(struct Node));

	if (n == NULL) {
		return 1;
	}

	if (levels[p].head == NULL) {
		levels[p].curr = levels[p].head = levels[p].tail = n;	

		levels[p].curr->next = levels[p].curr->prev = NULL;
	} else {
		levels[p].tail->next = n;

		n->prev = levels[p].tail;

		levels[p].tail = n;

		n->next = NULL;
	}

	n->pid = pid;

	n->quanta = prior_translation[p];

	return 0;
}

/*
 * Function to remove a process from the scheduler queue
 * @Param pid - the ID for the process/thread to be removed from the
 *      scheduler queue
 * @Return true/false response for if the removal was successful
 */
int removeProcess(int pid){
	int l;
	struct Node *n;

	if (find_node(pid, &n, &l)) {
		return 0;
	}

	if (n->prev == NULL) {
		if (n->next != NULL) {
			n->next->prev = NULL;
		}

		levels[l].head = n->next;

		if (levels[l].curr == n) {
			levels[l].curr = levels[l].head;
		}
	} else if (n->next == NULL) {
		if (n->prev != NULL) {
			n->prev->next = NULL;
		}

		levels[l].tail = n->prev;

		if (levels[l].curr == n) {
			levels[l].curr = levels[l].head;
		}
	} else {
		n->next->prev = n->prev;

		n->prev->next = n->next;

		if (levels[l].curr == n) {
			levels[l].curr = n->next;
		}
	}

	free(n);

	return 1;
}

/*
 * Function to get the next process from the scheduler
 * @Param time - pass by reference variable to store the quanta of time
 * 		the scheduled process should run for
 * @Return returns the thread id of the next process that should be 
 *      executed, returns -1 if there are no processes
 */
int nextProcess(int &time){
	if (!hasProcess()) {
		return -1;
	}

	int x, pid;
	struct Node *n;

	for (x = 0; x < kLevels; ++x, ++current) {
		if (levels[current].curr != NULL) {
			n = levels[current].curr;

			break;
		}
	}

	pid = n->pid;

	time = n->quanta;

	levels[current].curr = n->next;

	if (levels[current].curr == NULL) {
		levels[current].curr = levels[current].head;
	}	

	if (++current >= kLevels) {
		current = 0;
	}

	return pid;
}

/*
 * Function that returns a boolean 1 True/0 False based on if there are any 
 * processes still scheduled
 * @Return 1 if there are processes still scheduled 0 if there are no more 
 *		scheduled processes
 */
int hasProcess(){
	int x;

	for (x = 0; x < kLevels; ++x) {
		if (levels[x].head != NULL) {
			return 1;
		}	
	}

	return 0;
}
