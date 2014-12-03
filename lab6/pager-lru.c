/*
 * File: pager-lru.c
 * Author:       Andy Sayler
 *               http://www.andysayler.com
 * Adopted From: Dr. Alva Couch
 *               http://www.cs.tufts.edu/~couch/
 *
 * Project: CSCI 3753 Programming Assignment 4
 * Create Date: Unknown
 * Modify Date: 2012/04/03
 * Description:
 * 	This file contains an lru pageit
 *      implmentation.
 */

#include <stdio.h> 
#include <stdlib.h>
#include <limits.h>

#include "simulator.h"

void pageit(Pentry q[MAXPROCESSES]) { 
	/* This file contains the stub for an LRU pager */
	/* You may need to add/remove/modify any part of this file */

	/* Static vars */
	static int initialized = 0;
	static int tick = 1; // artificial time
	static int timestamps[MAXPROCESSES][MAXPROCPAGES];

	/* Local vars */
	int x, y;
	int lowest;
	int lowest_index;
	int page;
	int proctmp;
	int pagetmp;

	/* initialize static vars on first run */
	if(!initialized){
		for(proctmp=0; proctmp < MAXPROCESSES; proctmp++){
	    for(pagetmp=0; pagetmp < MAXPROCPAGES; pagetmp++){
				timestamps[proctmp][pagetmp] = 0; 
	    }
		}

		initialized = 1;
	}

	// Iterate through processes
	for (x = 0; x < MAXPROCESSES; ++x) {
		// Find each active process 
		if (q[x].active) {
			// Determine page
			page = q[x].pc/PAGESIZE;

			// Check if page is swapped out
			if (!q[x].pages[page]) {
				// Try to swap in page	
				if (!pagein(x, page)) {
					lowest = INT_MAX;
					lowest_index = 0;
	
					// Find the page with the oldest access time
					for (y = 0; y < q[x].npages; ++y) {
						if (page != y && timestamps[x][y] > 0 && timestamps[x][y] < lowest) {
							lowest = timestamps[x][y];
			
							lowest_index = y;
						}				
					}

					// Try to swap out page
					if (pageout(x, lowest_index)) {
						// Reset timestamp for page of process
						timestamps[x][lowest_index] = 0;
					}
				}	
			} else {	
				// Update the timestamp with latest time
				timestamps[x][page] = tick;			
			}
		}
	}   

	/* advance time for next pageit iteration */
	tick++;
} 
