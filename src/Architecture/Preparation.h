#ifndef __PREPARATION_H__
#define __PREPARATION_H__

#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <cstdlib>
#include <sys/timeb.h>
#include <unistd.h>

using namespace std;  

 //--------------------------------------------------------------------------------
// Showing error information
bool LackInput(int NumberofArgument)
{
	if(NumberofArgument<10)
	{
		printf("\nPlease Input 9 mparameters as required...\n\n");
		printf("---------INPUT FORMAT------------------\n");
		cout<<"./progm---path2table---path2input---#STATE---#SYMBOL"<<endl;
		cout<<"---START_STATE---TOKEN_TYPE---ALGORITHM_VERSION"<<endl<<endl;
		sleep(1);
		return true;
	}
	else
		return false;
}

#endif
