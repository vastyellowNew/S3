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
#include <math.h>
#include <sched.h>
#include <errno.h>
#include <unistd.h>

#include "Offline.h"
#include "seq.cpp"


using namespace std;		

int main(int argv, char* argc[])
{
	char* dfafile;
	char* inputfile;
	char* TestFile;

	//Initialization
	dfafile=argc[1]; 				// The dfa transtion file name
	inputfile=argc[2]; 				// The input file name
	TestFile=argc[3]; 
	state_num=atoi(argc[4]);
	symbol_num=atoi(argc[5]);

	//Load operation
    	load_input(inputfile);			// Getting the input string, constructing the int-type string
	load_dfa_table(dfafile);		// Loading 2D state-major table, constructing the T3 table
	READ_test(TestFile);


	int count1;

	for(count1=0;count1<pair_num;count1++)
		convl[count1]=transition(state_store1[count1], state_store2[count1]);
	//cout<<pair_num<<endl;

	ofstream out;
	out.open("DetailConvergence.txt",ios::app);
	for(count1=0;count1<pair_num;count1++)
		out<<state_store1[count1]<<" "<<state_store2[count1]<<" "<<convl[count1]<<endl;

	// Free the memory
	delete []T;
	delete []input;
	return 0;
}
