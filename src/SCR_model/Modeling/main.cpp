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

using namespace std;		

int main(int argv, char* argc[])
{
	char* file1;
	char* file2;
	char* file3;

	file1=argc[1];
	file2=argc[2];
	file3=argc[3];

	ifstream in;
	in.open(file1);
	ofstream out;
	out.open(file3);
	int temp1;
	int avoidr=0;

	while(in)
	{
		int n1;
		long l1;
		long totallength=0;
	
		int* pair1;
		int* pair2;

		in>>n1>>l1;
		if(n1==256&&avoidr==30)
			break;
		
		pair1=new int [n1-1];
		pair2=new int [n1-1];
		
		for(temp1=0;temp1<n1-1;temp1++)
		{
			in>>pair1[temp1]>>pair2[temp1];	
			if(pair1[temp1]==pair2[temp1])
			{
				totallength+=0;
				continue;
			}
			long smaller=0;
			long larger=0;
			long temptotal=0;

			ifstream in1;
			in1.open(file2);
			while(in1)
			{
				int d1, d2;
				long dlength;
				in1>>d1>>d2>>dlength;
				if(d1==pair1[temp1]&&d2==pair2[temp1])
				{
					if(dlength<l1)
					{
						smaller++;
						temptotal+=dlength;
					}
					else
						larger++;
				}
			}
			in1.close();

			totallength+=(temptotal+l1*larger)/(smaller+larger);
		
/*	
			if(smaller>larger)
				totallength+=(temptotal)/(smaller);
			else
				totallength+=l1;
*/			
			//cout<<n1<<" "<<smaller<<" "<<larger<<endl;
		}
		out<<n1<<" "<<totallength<<endl;
		if(n1==256)
			avoidr++;

	}
	in.close();
	out.close();
	return 0;
}

