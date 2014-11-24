#include "schedule.h"
#include <stdlib.h>

// Node structure for a doubly linked list
struct Node {
	int pid;
	int quanta;
	Node *next;
	Node *prev;
};

// Doubly linked list structure
struct Queue {
	Node *curr;
	Node *head;
	Node *tail;
};

// Defines the number of levels
const int kLevels = 4;

// Current level of the queue
int current;
// Multilevel queue
struct Queue levels[kLevels];
// Translates priority into time quantum for the scheduler
int priority_translation[kLevels];

// Returns 0 on success or 1 on failure
// Helper function to find a node in the multilevel queue
int find_node(int pid, struct Node **node, int *level) {
	int x;
	struct Node *n;

	// Iterates over the levels	
	for (x = 0; x < kLevels; ++x) {
		// Begin at the head of the current level
		n = levels[x].head;

		// Look for a node with the matching pid
		while (n) {
			if (n->pid == pid) {
				*node = n;

				*level = x;

				return 0;
			}

			// Step the queue forward
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

	// Begin at level 0
	current = 0;

	// Initializes the priority to quantum translations
	for (x = kLevels-1; x >= 0; --x) {
		priority_translation[x] = kLevels-x;
	}

	// Initialize all levels to NULL
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
	// Adjust priority for index
	int p = priority - 1;
	// Create a new node
	struct Node *n = (struct Node *)calloc(1, sizeof(struct Node));

	// Check for error allocating memory
	if (n == NULL) {
		return 1;
	}

	// The node is either set as the head or attached to the tail
	// of the queue
	if (levels[p].head == NULL) {
		levels[p].curr = levels[p].head = levels[p].tail = n;	

		levels[p].curr->next = levels[p].curr->prev = NULL;
	} else {
		levels[p].tail->next = n;

		n->prev = levels[p].tail;

		levels[p].tail = n;

		n->next = NULL;
	}

	// Set pid for the node
	n->pid = pid;

	// Set the time quantum for the node
	n->quanta = priority_translation[p];

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

	// Find the node we want to remove
	if (find_node(pid, &n, &l)) {
		return 0;
	}

	// Decide if we're removing the head, tail or middle node in a queuek
	if (n->prev == NULL) {
		// Remove the node from the head
		if (n->next != NULL) {
			n->next->prev = NULL;
		}

		// Set the new head
		levels[l].head = n->next;

		// Adjust the current pointer
		if (levels[l].curr == n) {
			levels[l].curr = levels[l].head;
		}
	} else if (n->next == NULL) {
		// Remove the node from the tail
		if (n->prev != NULL) {
			n->prev->next = NULL;
		}

		// Set the new tail
		levels[l].tail = n->prev;

		// Adjust the current pointer
		if (levels[l].curr == n) {
			levels[l].curr = levels[l].head;
		}
	} else {
		// Remove the node
		n->next->prev = n->prev;

		n->prev->next = n->next;

		// Adjust the current pointer
		if (levels[l].curr == n) {
			levels[l].curr = n->next;
		}
	}

	// Free the node
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
	// Check if there are still processes
	if (!hasProcess()) {
		return -1;
	}

	int x, pid;
	struct Node *n;

	// Find the next level that has a valid node
	for (x = 0; x < kLevels; ++x, ++current) {
		if (levels[current].curr != NULL) {
			n = levels[current].curr;

			break;
		}
	}

	// Get the pid to return
	pid = n->pid;

	// Return the time quantum
	time = n->quanta;

	// Step the queue forward
	levels[current].curr = n->next;

	// Hit end of the queue reset to the head
	if (levels[current].curr == NULL) {
		levels[current].curr = levels[current].head;
	}	

	// Step the level forward for the round robin
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

	// Check each level for a valid node
	for (x = 0; x < kLevels; ++x) {
		if (levels[x].head != NULL) {
			return 1;
		}	
	}

	return 0;
}
