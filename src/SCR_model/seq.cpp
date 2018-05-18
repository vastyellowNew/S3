#include <stdio.h>
#include <stdint.h>
#include <string>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>  
#include <algorithm>
#include <math.h>
#include "Offline.h"

using namespace std;

long transition(int state1, int state2)
{
	long i;
	int current1, current2;
	int symbol;
	
	current1=state1;
	current2=state2;

	for(i=0;i<length;i++)
	{
		symbol=input[i];
		current1=T[current1][symbol];
		current2=T[current2][symbol];
		
		if(current1==current2)
			break;
	}

	//cout<<state1<<" "<<state2<<" "<<i<<endl;
	
	return i;
}
