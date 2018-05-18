#ifndef __OFFLINE_H__
#define __OFFLINE_H__

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <cstdlib>
#include <sys/timeb.h>
#include <unistd.h>
#include <ctime>
#include <climits>
#include <iostream>
#include <fstream>
#include <vector>  
#include <algorithm>

using namespace std;  

//--------------------------------------------
int** T;		// DFA transition table
int state_num;
int symbol_num;
int* input;
long length;
int* state_store1;
int* state_store2;
int pair_num;
long* convl;

//--------------------------------------------

void load_dfa_table(char* dfafile1)
{
	int i,j;
	ifstream dfafile;
	dfafile.open(dfafile1);

	T = new int* [state_num];
	for(i=0;i<state_num;i++)
		T[i]=new int [symbol_num];

	for(i=0;i<state_num;i++)
		for(j=0;j<symbol_num;j++)
		{
			int temp;
			dfafile>>temp;
			if(temp==(-1))
				temp=0;
			T[i][j]=temp;
		}
	dfafile.close();
}	

//-------------------------------------------------------
// load input to memory 
void load_input(char* inputfile) 
{
	int temp=0;
	long i=0;
	int maptable[256];
	ifstream in;
	string str;

	in.open(inputfile);
	length=0;
	while(getline(in,str))
	{
		length=str.size()+length;
	}
	in.close();

	for(temp=0;temp<256;temp++)
		maptable[temp]=temp;

	//--------------------------------------
	// constructing the input string with int type
	in.open(inputfile);
	input = new int [length];	
	while(in)
	{
		char chara;
		in>>chara;
		if((int)chara>255||(int)chara<0)
			continue;
		input[i]=maptable[(int)chara];
		i++;
	}
	in.close();
	length=i;
	//---------------------------------------

}

void READ_test(char* testfile)
{
	ifstream in;
	in.open(testfile);
	int record=0;
	int* tempstore1;
	int* tempstore2;
	tempstore1=new int [60480];
	tempstore2=new int [60480];
	tempstore1[0]=0;
	tempstore2[0]=0;

	int temp1;
	int temp2;

	
	while(in)
	{
		int n1;
		long l1;
	
		int* pair1;
		int* pair2;

		in>>n1>>l1;
		
		pair1=new int [n1-1];
		pair2=new int [n1-1];
		
		for(temp1=0;temp1<n1-1;temp1++)
		{
			in>>pair1[temp1]>>pair2[temp1];	
			if(pair1[temp1]==pair2[temp1])
			{
				//cout<<n1<<" "<<l1<<" "<<pair1[temp1]<<endl;
				continue;
			}

			bool Exist=false;	
			for(temp2=0;temp2<record;temp2++)
			{
				if(pair1[temp1]==tempstore1[temp2] && pair2[temp1]==tempstore2[temp2])
				{
					Exist=true;
					break;
				}
			}
			if(Exist==false)
			{
				tempstore1[record]=pair1[temp1];
				tempstore2[record]=pair2[temp1];
				record++;
			}
		}
	}

	state_store1=new int [record];
	state_store2=new int [record];
	convl=new long [record];
	pair_num=record;
	for(temp1=0;temp1<record;temp1++)
	{
		state_store1[temp1]=tempstore1[temp1];
		state_store2[temp1]=tempstore2[temp1];
	}
	delete []tempstore1;
	delete []tempstore2;
}

#endif
