/*
 * File: pager-predict.c
 * Author:       Andy Sayler
 *               http://www.andysayler.com
 * Adopted From: Dr. Alva Couch
 *               http://www.cs.tufts.edu/~couch/
 *
 * Project: CSCI 3753 Programming Assignment 4
 * Create Date: Unknown
 * Modify Date: 2012/04/03
 * Description:
 * 	This file contains a predictive pageit
 *      implmentation.
 */

// Sample run ***************************************************
//
// ********** WARNING *************
// Takes about a minute and a half to run
// ********** WARNING *************
// 
// Architecture:          x86_64
// CPU op-mode(s):        32-bit, 64-bit
// Byte Order:            Little Endian
// CPU(s):                8
// On-line CPU(s) list:   0-7
// Thread(s) per core:    2
// Core(s) per socket:    4
// Socket(s):             1
// NUMA node(s):          1
// Vendor ID:             GenuineIntel
// CPU family:            6
// Model:                 42
// Model name:            Intel(R) Core(TM) i7-2600K CPU @ 3.40GHz
// Stepping:              7
// CPU MHz:               1637.578
// CPU max MHz:           3800.0000
// CPU min MHz:           1600.0000
// BogoMIPS:              6787.62
// Virtualization:        VT-x
// L1d cache:             32K
// L1i cache:             32K
// L2 cache:              256K
// L3 cache:              8192K
// NUMA node0 CPU(s):     0-7
//
// 00000000: random seed 876014295
// 00000000: using 20 processors
// 00233310: simulation ends
// 00233310: 650809 blocked cycles
// 00233310: 1758270 compute cycles
// 00233310: ratio blocked/compute=0.370142
// 
// real	1m24.031s
// user	1m24.067s
// sys	0m0.000s

// this paging strategy uses a Markov chain to
// predict the next page needed by the current 
// process. It also uses LRU for page replacement

#include <stdio.h> 
#include <stdlib.h>
#include <limits.h>

#include "simulator.h"

// uncomment to generate the transition matrix
//#define GEN_TRANS_MATRIX

// function will multiple the matrix m1 by m2
// and copy the results into m2
void matrix_update(double *m1, double *m2) {
	static double temp[MAXPROCPAGES][MAXPROCPAGES];
	
	int x, y, z;
	double total;

	for (x = 0; x < MAXPROCPAGES; ++x) {
		for (y = 0; y < MAXPROCPAGES; ++y) {
			total = 0.0;

			for (z = 0; z < MAXPROCPAGES; ++z) {
				total += m1[x*MAXPROCPAGES+z] * m2[z*MAXPROCPAGES+y];
			}

			temp[x][y] = total;
		}
	}	

	for (x = 0; x < MAXPROCPAGES; ++x) {
		for (y = 0; y < MAXPROCPAGES; ++y) {
			m2[x*MAXPROCPAGES+y] = temp[x][y];
		}
	}
}

// returns the predicted page
// function will attempt to predict the next page
// base on the current transition matrix
int predict_page(double *m1, double *m2) {
	static double temp[MAXPROCPAGES];

	int x, y;
	double max;
	double total;
	int max_index;

	for (x = 0; x < MAXPROCPAGES; ++x) {
		total = 0.0;

		for (y = 0; y < MAXPROCPAGES; ++y) {
			total += m2[y] * m1[x*MAXPROCPAGES+y];
		}
	
		temp[x] = total;
	}

	max = 0.0;

	// find the maximum value which denotes the 
	// predicted page index
	for (x = 0; x < MAXPROCPAGES; ++x) {
		if (temp[x] > max) {
			max = temp[x];

			max_index = x;
		}
	}

	return max_index;
}

void pageit(Pentry q[MAXPROCESSES]) { 
	/* This file contains the stub for a predictive pager */
	/* You may need to add/remove/modify any part of this file */

	/* Static vars */
	static int initialized = 0;
	static int tick = 1; // artificial time
	static double trans_matrix[MAXPROCPAGES][MAXPROCPAGES] = // base transition matrix
		{{0.9980, 0.0020, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000},
		{0.0000, 0.9980, 0.0020, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000},
		{0.0000, 0.0000, 0.9980, 0.0019, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000},
		{0.0001, 0.0000, 0.0000, 0.9980, 0.0016, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0003, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000},
		{0.0000, 0.0000, 0.0000, 0.0000, 0.9975, 0.0025, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000},
		{0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.9975, 0.0025, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000},
		{0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.9975, 0.0025, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000},
		{0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.9975, 0.0025, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000},
		{0.0011, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.9973, 0.0015, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000},
		{0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.9993, 0.0007, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000},
		{0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.9993, 0.0007, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000},
		{0.0003, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.9993, 0.0004, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000},
		{0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.9996, 0.0004, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000},
		{0.0006, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0011, 0.0000, 0.0000, 0.0000, 0.9981, 0.0002, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000},
		{0.0045, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.9955, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000},
		{0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000},
		{0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000},
		{0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000},
		{0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000},
		{0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000}};
	static double proc_matrix[MAXPROCESSES][MAXPROCPAGES][MAXPROCPAGES]; // current transition matrix for each process
	static int timestamps[MAXPROCESSES][MAXPROCPAGES]; // current timestamps for each page
#ifdef GEN_TRANS_MATRIX
	static int last_page[MAXPROCPAGES];
	static int gen_trans_matrix[MAXPROCPAGES][MAXPROCPAGES];
#endif

	/* Local vars */
	int page;
	int x, y, z;
	int lowest;
	int lowest_index;
	int page_prediction;
	double prediction[MAXPROCPAGES];

	/* initialize static vars on first run */
	if(!initialized){
		for (x = 0; x < MAXPROCESSES; ++x) {
			for (y = 0; y < MAXPROCPAGES; ++y) {
				timestamps[x][y] = 0;

				for (z = 0; z < MAXPROCPAGES; ++z) {
					proc_matrix[x][y][z] = trans_matrix[y][z];
				}
			}
		}

#ifdef GEN_TRANS_MATRIX
		for (x = 0; x < MAXPROCPAGES; ++x) {
			last_page[x] = -1;

			for (y = 0; y < MAXPROCPAGES; ++y) {
				gen_trans_matrix[x][y] = 0;
			}
		}
#endif

		/* Init complex static vars here */
		initialized = 1;
	}

	// iterate through active processes
	for (x = 0; x < MAXPROCESSES; ++x) {
		if (q[x].active) {
			// determine current page
			page = q[x].pc/PAGESIZE;

			// check if page is swapped out
			if (!q[x].pages[page]) {
				// attempt to swap page in
				if (!pagein(x, page)) {
					// find the LRU page to swap out
					lowest = INT_MAX;
					lowest_index = -1;

					for (y = 0; y < MAXPROCPAGES; ++y) {
						if (y != page && timestamps[x][y] > 0 && timestamps[x][y] < lowest) {
							lowest = timestamps[x][y];

							lowest_index = y;
						}
					}

					// if we swap the page out reset timestamp
					if (pageout(x, lowest_index)) {
						timestamps[x][lowest_index] = 0;
					}
				} else {
					// update timestamp to current time
					timestamps[x][page] = tick;

#ifdef GEN_TRANS_MATRIX
					// update matrix if no on first run
					if (last_page[x] != -1) {
						++gen_trans_matrix[last_page[x]][page];
					}
		
					// set current page as last page for next iteration
					last_page[x] = page;
#endif
				}
			} else {
				// update timestamp to current time
				timestamps[x][page] = tick;

#ifdef GEN_TRANS_MATRIX
				// update matrix
				++gen_trans_matrix[page][page];
#endif
			}

			// update transition matrix for Markov chain
			// x^t+1 = x^t * trans_matrix^t
			matrix_update(&trans_matrix[0][0], &(proc_matrix[x])[0][0]);	

			// load input matrix
			for (y = 0; y < MAXPROCPAGES; ++y) {
				if (y == page) {
					prediction[y] = 1.0;
				} else {
					prediction[y] = 0.0;
				}
			}

			// make prediction
			page_prediction = predict_page(&(proc_matrix[x])[0][0], &prediction[0]);

			// attempt to page in predicted page
			// check for conflicts
			if (!q[x].pages[page_prediction] && page != page_prediction) {
				if (!pagein(x, page_prediction)) {
					// pageout based on LRU
					lowest = INT_MAX;
					lowest_index = -1;

					for (y = 0; y < MAXPROCPAGES; ++y) {
						if (y != page && y != page_prediction && timestamps[x][y] > 0 && timestamps[x][y] < lowest) {
							lowest = timestamps[x][y];

							lowest_index = y;
						}
					}

					if (pageout(x, lowest_index)) {
						timestamps[x][lowest_index] = 0;
					}
				}
			}
		}
	}

	/* advance time for next pageit iteration */
	tick++;

#ifdef GEN_TRANS_MATRIX
	for (x = 0; x < MAXPROCESSES; ++x) {
		if (q[x].active) {
			return;
		}
	}

	int row_total;

	printf("{");

	for (x = 0; x < MAXPROCPAGES; ++x) {
		printf("{");

		row_total = 0;

		for (y = 0; y < MAXPROCPAGES; ++y) {
			row_total += gen_trans_matrix[x][y];
		}

		for (y = 0; y < MAXPROCPAGES; ++y) {
			if (gen_trans_matrix[x][y] == 0) {
				printf("%.4f", 0.0);	
			} else {
				printf("%.4f", (double)gen_trans_matrix[x][y]/(double)row_total);
			}
	
			printf("%s", (y == MAXPROCPAGES-1) ? "" : ", ");
		}

		printf("%s\n", (x == MAXPROCPAGES-1) ? "}}" : "},");
	}
#endif
}
