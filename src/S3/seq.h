/* 
 * This file only implements one kinds of base line and multil-core seq version, 
 * which is  2d-state.
 * The output operation is checking operation, 
 * which means checking the current state for matching.
 * This file is edited by J.Q. on 07/18/2016.
*/

#include <stdio.h>
#include <stdint.h>
#include <string>
#include <cstdlib>
#include <sys/timeb.h>
#include <ctime>
#include <climits>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>  
#include <algorithm>
#include <math.h>
#include "Offline.h"

using namespace std;		

void base3()
{
	int current, symbol;
	long i;
	
	current=start_state;
	for (i=0;i<len;i++)
	{
		symbol=input[i];
		current = T3[current][symbol];
		//base_store[i]=current;		//Output store
		
		/* Check accept */
		if(acceptstate[current])
			match++;
	}
	cout<<"The final state is "<<current<<endl;
}

//-----------------Multi-thread versions--------------------------
//2d-state
void* seq3(void *threadid)
{
	long tid =  (long)threadid;
    	long i;
	int current,symbol;
	int pmatch=0;
	current=pthread_predict[tid];

	long pbound=(tid+1)*charlen;
	long starti=tid*charlen;
	for(i=starti;i<pbound;i++)
	{
		symbol=input[i];
		current=T3[current][symbol];
		//seq_store[i]=current; 			//Output store
		
		//check match
		if(acceptstate[current])
			pmatch++;
	}
	pthread_mutex_lock(&mutex1);
	match=match+pmatch;
	pthread_mutex_unlock(&mutex1);

	pthread_final[tid]=current;
    	printf("%ld is running on CPU %d\n", tid, sched_getcpu());
	pthread_exit((void*)threadid);
}
