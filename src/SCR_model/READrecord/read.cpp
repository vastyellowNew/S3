#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <cstdlib>
#include <unistd.h>
#include <ctime>
#include <climits>
#include <iostream>
#include <fstream>
#include <vector>  
#include <algorithm>

using namespace std;

int main(int argc, char* argv[])
{
	char* filename;
	ifstream in;
	ofstream out;
	int repeattime;

	filename = argv[1];
	repeattime = atoi(argv[2]);
	in.open(filename);
	out.open("Information.txt");

	int n1;
	long l1;
	double p1;
	int temp;
	int avoidr=0;

	while(in)
	{
		if(n1==repeattime&&avoidr==30)
			break;

		in>>n1>>l1>>p1;
		int* wrong;
		int* correct;
		int useless;
		string usestring;
		wrong=new int [n1];
		correct=new int [n1];
		for(temp=0;temp<n1;temp++)
			in>>wrong[temp];
		for(temp=0;temp<n1;temp++)
			in>>correct[temp];
		
		in>>useless;
		getline(in, usestring);
		in>>useless;
		getline(in, usestring);


		out<<n1<<" "<<l1<<endl;
		for(temp=1;temp<n1;temp++)
			out<<wrong[temp]<<" "<<correct[temp-1]<<endl;
		out<<endl;

		if(n1==repeattime)
			avoidr++;
	}
	in.close();
	out.close();
	return 0;
}	
