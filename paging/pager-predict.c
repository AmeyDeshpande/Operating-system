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

#include <stdio.h> 
#include <stdlib.h>

#include "simulator.h"

void pageit(Pentry q[MAXPROCESSES]) {

	/* This file contains the stub for a predictive pager */
	/* You may need to add/remove/modify any part of this file */

	/* Static vars */
	static int initialized = 0;
	static int tick = 1; // artificial time
  int proc;
  int pc;
  int page;
  int i;
	/* Local vars */
  for (proc = 0; proc < MAXPROCESSES; proc++) {
    pc = q[proc].pc;
    page = pc / PAGESIZE;
    for(i=0;i<7;i++)
    {      
        if(i!=page)
          pageout(proc, i);
      
    }
    //timestamps[proc][page] = tick;
          if(!q[proc].pages[page]) 
          {
        /* Try to swap in */
            if(!pagein(proc,page)) 
            {    
                if(0 != page) 
                {
                  if(pageout(proc,0))
                  {
                  }                 
                }
                else
                {
                  if(pageout(proc,1))
                  {
                  }                 
                }                
            }
          }
          //printf("%d",proc);
  }
	/* initialize static vars on first run */
	if (!initialized) {
		/* Init complex static vars here */

		initialized = 1;
	}

	/* TODO: Implement Predictive Paging */
	//fprintf(stderr, "pager-predict not yet implemented. Exiting...\n");
	//exit(EXIT_FAILURE);

	/* advance time for next pageit iteration */
	tick++;
}
