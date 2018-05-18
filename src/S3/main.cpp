/*
	* This Project aims to figure out the relationship among speedup, energy and configuration. 	
	* ATTENTION: Actually the whole codes are based on speculation. 
	* Which means, all version need check reprocessing.
 	*
	* This version is pthread/multithread version (with bonding and only testing the 2D-state version). 
	*
	* The inputs you have to offer are dfa file name, input file name, and
	* number of states, number of symbols, number of species for speculation algorithm, 
	* the start state for seq version, the kind of benchmark, the mode,
	* and the number of cores and the number of threads.
	* 
	* edited by J.Q on 12/12/2016.
	* No need for PAPI in this version.
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <cstdlib>
#include <sys/timeb.h>
#include <ctime>
#include <climits>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>  
#include <algorithm>
#include <smmintrin.h> 		// sse4.2
#include <immintrin.h>   	// avx
#include <math.h>
#include <pthread.h>
#include <sched.h>
#include <errno.h>
#include <unistd.h>

#include "Offline.h"
#include "Preparation.h" 
#include "seq.h"
//#include "PapiInitialization.h"
//#include "spec_nonSIMD.cpp"
//#include "spec_avx.cpp"
//#include "spec_avxunroll.cpp"
//#include "sse.cpp"

using namespace std;		

int main(int argv, char* argc[])
{
	char* dfafile;
	char* inputfile;
	char* acceptfile;
	int repeat;
	ofstream outfile1,outfile2;		// Files which are used to store the time spend

	// Error situation
	if(LackInput(argv))
		return -1;

	//Initialization
	dfafile=argc[1]; 				// The dfa transtion file name
	inputfile=argc[2]; 				// The input file name
	acceptfile=argc[3];				// The accept file name
	state_num=atoi(argc[4]);		// The number of states (global)
	symbol_num=atoi(argc[5]);		// The number of symbols (global)
	start_state=atoi(argc[6]);		// Start state (global)
	KIND=atoi(argc[7]);				// Choose the kind of benchmark (token version, global) 
	mode=atoi(argc[8]);				// Choose the mode (which means processing version, global)
	int TOTAL_NUM_CORES=atoi(argc[9]);
	int TOTAL_NUM_THREASS=atoi(argc[10]);
	int NUM_CORES;					// The number of logical processing units (not global)
	int NUM_THREADS;				// The number of user-defined threads (not global)

	//Load operation
    load_input(inputfile);			// Getting the input string, constructing the int-type string
	load_dfa_table(dfafile,3);		// Loading 2D state-major table, constructing the T3 table
	load_accept_file(acceptfile);	// Loading the accept state file, constructing the accept state array

	for (NUM_CORES=1;NUM_CORES<=TOTAL_NUM_CORES;NUM_CORES++)
	{
		if (TOTAL_NUM_CORES!=TOTAL_NUM_THREASS)
			return -1;
		NUM_THREADS=NUM_CORES;
		spec_split1=NUM_THREADS;	// spec_split1 can be used as a global variable

		// Determine and show the length of input	
		len=(len/(NUM_THREADS))*NUM_THREADS; 	// For parallel running convinient testing
		cout<<endl<<"The input length is "<<len<<endl<<endl;

		//PTHREAD--------------------------------
		charlen = len/NUM_THREADS;  // global variable 'charlen'
		/* pthread version variables */
		int rc;
		long t;
		int temptb;
		pthread_t* threads;			
		threads=(pthread_t*)malloc(sizeof(pthread_t)*NUM_THREADS);
		/* thread binding variables */
		cpu_set_t* cpu;
		cpu=(cpu_set_t*)malloc(sizeof(cpu_set_t)*NUM_CORES);
		//PTHREAD-------------------------------

		for (repeat=0;repeat<1;repeat++)	// The repeat time can be changed if needed
		{
			/* Time varibale */
			long cmp1,cmp1s;
			struct timeb startTime1, endTime1;		// For parallel running time measurement
			struct timeb startTime1s, endTime1s;		// For reprocessing time measurement

			ofstream out_time;
			out_time.open("Total_Average_running_time.txt",ios::app);

			//----------------------base line--------------------------------
			if (mode==0||NUM_THREADS==1)			// mode=0 means only checking seq
			{
				//base_store=new int [len];
				match=0;	// Global variables 'match'

				ftime(&startTime1);
				base3();
				ftime(&endTime1);
		
				cmp1=(endTime1.time-startTime1.time)*1000+
						(endTime1.millitm-startTime1.millitm);
				cout<<endl<<"Total Process time (ms)"<<cmp1<<endl;
				cout<<"Match time is "<<match<<endl<<endl;

				out_time<<NUM_THREADS<<" "<<cmp1<<endl;
				out_time.close();	
			}

			//----------seq parallel version------------------------------------------
			if (mode==1&&NUM_THREADS>1)
			{
				//seq_store=new int [len];
				// Predict opearation
				//pthread_predictt(NUM_THREADS);
				pthread_predict_complex(NUM_THREADS);	

				for(int m1=0;m1<NUM_THREADS;m1++)
				{
					cout<<"Predict(start) state for "<<m1;
					cout<<" pthread is "<<pthread_predict[m1]<<endl;
				}	
				cout<<endl;

				pthread_final=new int [NUM_THREADS];
		
				//--------seq version 2d-state------------
				match=0;
				mutex1 = PTHREAD_MUTEX_INITIALIZER;

				// Process
				ftime(&startTime1);

				// thread binding
				for(t=0; t<NUM_CORES; t++)
				{
	    				CPU_ZERO(&cpu[t]);
	    				CPU_SET(t, &cpu[t]);
				}	
	   			for(t=0;t<NUM_THREADS;t++)
				{
	     				printf("In main: creating thread %ld\n", t);
	     				rc = pthread_create(&threads[t], NULL, seq3, (void *)t);
	     			if (rc)
					{
	       				printf("ERROR; return code from pthread_create() is %d\n", rc);
	       				exit(-1);
	       			}
				
				// thread binding
	    			temptb = pthread_setaffinity_np(threads[t], 
	    						sizeof(cpu_set_t), &cpu[t%NUM_CORES]);
	     		}

				for(t=0; t<NUM_THREADS; t++)
	        			pthread_join(threads[t], NULL);
				ftime(&endTime1);

				ftime(&startTime1s);
				//thread_Recompute(NUM_THREADS,1);
				thread_Recompute_match(NUM_THREADS, 3);
				ftime(&endTime1s);

				// Results shown
				for (t=0;t<NUM_THREADS;t++)
					cout<<"The final state of thread "<<t<<" is "<<pthread_final[t]<<endl;
				for (t=0;t<NUM_THREADS;t++)
					cout<<"The REAL final state of thread "<<t<<" is "<<WHOLEFINAL[t]<<endl;

				//check match 
				cout<<"Match time is "<<match<<endl;

				// Time spent
				cmp1=(endTime1.time-startTime1.time)*1000+
					(endTime1.millitm-startTime1.millitm);
				cmp1s=(endTime1s.time-startTime1s.time)*1000+
					(endTime1s.millitm-startTime1s.millitm);

				cout<<"Total time (ms) "<<cmp1+cmp1s<<endl;					
				cout<<"Parallel process time (ms) "<<cmp1<<endl;
				cout<<"Reprocess time (ms) "<<cmp1s<<endl;

				out_time<<NUM_CORES<<" "<<cmp1+cmp1s<<endl;
				out_time.close();	
				
				outfile1.open("Total_Parallel_time.txt", ios::app);
				outfile1<<NUM_CORES<<" "<< cmp1 << endl;
				outfile1.close();
			
				outfile1.open("Total_Reprocess_time.txt",ios::app);
				outfile1 <<NUM_CORES<<" "<< cmp1s <<endl;
				outfile1.close();

				cout<<endl;
			}

			if (mode==1&&NUM_CORES>1&&repeat==0)
				reprocess_information();
			
			delete []pthread_final;
			delete []pthread_predict;		
			cout<<"Done repeat "<<repeat<<" at core "<<NUM_THREADS<<endl;
		}

	}

	// Free the memory
	delete []T3;
	delete []input;
	return 0;
}
