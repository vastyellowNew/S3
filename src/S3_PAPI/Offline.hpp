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
#include <smmintrin.h> 		// sse4.2
#include <immintrin.h>   	// avx
#include <pthread.h>

using namespace std;  

//-------------The global variables for the seq and normal sse versions-------------------	
// input params 
int 	state_num;		// number of states
int 	symbol_num;		// number of symbols
int 	spec_split1;	// number of species for spec version (for global use)
int   	start_state;	// the start state
long 	len;  			// input array length
int 	KIND;			// the Benchmark-token kind chosen
int 	mode;			// the running mode chosen

int*	input __attribute__ ((aligned (32)));
int*	predict_state;	// parameter for spec
bool* 	acceptstate;
// results stored and match time
int* 	base_store;
int* 	seq_store;
int		match=0;

// transition table for seq version
int*	T1 __attribute__ ((aligned (32)));  // dfa table 1d-state
int*	T2 __attribute__ ((aligned (32)));  // dfa table 1d-symbol
int**	T3 __attribute__ ((aligned (32)));  // dfa table 2d-state
int**	T4 __attribute__ ((aligned (32)));  // dfa table 2d-symbol

// pthread parameter
int 	charlen;
int*	storestate;
int* 	pthread_predict;
int* 	pthread_final;
pthread_mutex_t mutex1;
long* rep;	// Reprocess length for each chunk
int* WHOLEFINAL;

//-------------Some instructions dealing with global variables for seq version------------
// load table
void load_dfa_table(char* dfafile1, int choose)
{
	int i,j;
	ifstream dfafile;
	dfafile.open(dfafile1);

	if (choose==1)			// load for T1 
	{
		T1= new int [state_num*symbol_num];
		for(i=0;i<state_num;i++)
			for(j=0;j<symbol_num;j++)
			{
				int temp;
				dfafile>>temp;
				if(temp==(-1))
					temp=start_state;
				T1[i*symbol_num+j]=temp;
			}
	}
	else if (choose==2)		// load for T2 
	{
		T2 = new int [symbol_num*state_num];
		for(i=0;i<state_num;i++)
			for(j=0;j<symbol_num;j++)
			{
				int temp;
				dfafile>>temp;
				if(temp==(-1))
					temp=start_state;
				T2[i+j*state_num]=temp;
			}
	}
	else if (choose==3)		// load for T3 
	{
		T3 = new int* [state_num];
		for(i=0;i<state_num;i++)
			T3[i]=new int [symbol_num];

		for(i=0;i<state_num;i++)
			for(j=0;j<symbol_num;j++)
			{
				int temp;
				dfafile>>temp;
				if(temp==(-1))
					temp=start_state;
				T3[i][j]=temp;
			}
	}
	else if (choose==4)		// load for T4 
	{
		T4 = new int* [symbol_num];
		for(i=0;i<symbol_num;i++)
			T4[i]=new int [state_num];

		for(i=0;i<state_num;i++)
			for(j=0;j<symbol_num;j++)
			{
				int temp;
				dfafile>>temp;
				if(temp==(-1))
					temp=start_state;
				T4[j][i]=temp;
			}
	}
	dfafile.close();
}	

//-------------------------------------------------------
// load input to memory 
void load_input(char* inputfile) 
{
	int temp=0;
	ifstream in;
	string str;
	long i=0;
	int maptable[256];

	in.open(inputfile);
	len=0;
	while(getline(in,str))
	{
		len=str.size()+len;
	}
	in.close();

	if(KIND==0)		//DIV, 01 symbols
	{
		maptable['0']=0;
		maptable['1']=1;
	}
	else if (KIND==1)	//SNORT, 256 symbols
		for(temp=0;temp<256;temp++)
			maptable[temp]=temp;
	else if (KIND==2)	//DNA, 4 symbols
	{
		maptable['A']=0;
		maptable['T']=1;
		maptable['C']=2;
		maptable['G']=3;
	}
	else if (KIND==3)	//PROTN, 20 symbols
	{
		maptable['A']=0;
		maptable['C']=1;
		maptable['D']=2;
		maptable['E']=3;
		maptable['F']=4;
		maptable['G']=5;
		maptable['H']=6;
		maptable['I']=7;
		maptable['K']=8;
		maptable['L']=9;
		maptable['M']=10;
		maptable['N']=11;
		maptable['P']=12;
		maptable['Q']=13;
		maptable['R']=14;
		maptable['S']=15;
		maptable['T']=16;
		maptable['V']=17;
		maptable['W']=18;
		maptable['Y']=19;
	}
	else if (KIND==4)		//EVENODD
	{
		maptable['a']=0;
		maptable['b']=1;
		maptable['c']=2;
		maptable['d']=3;
	}

	else if (KIND==5)		// different clarissa
	{	
		for (int token=0;token<256;token++)
		{
			if(token==(int)'y')
				maptable[token]=0;
			else if(token==(int)'o')
				maptable[token]=1;
			else if(token==(int)'u')
				maptable[token]=2;
			else if(token==(int)'l')
				maptable[token]=3;
			else if(token==(int)'i')
				maptable[token]=4;
			else if(token==(int)'k')
				maptable[token]=5;
			else if(token==(int)'e')
				maptable[token]=6;
			else if(token==(int)'a')
				maptable[token]=7;
			else if(token==(int)'p')
				maptable[token]=8;
			else if(token==(int)',')
				maptable[token]=9;
			else if(token==(int)'.')
				maptable[token]=10;

			else
				maptable[token]=11;
		}
	}
//--------------------------------------
	// constructing the input string with int type
	in.open(inputfile);
	input = new int [len];	
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
	len=i;
//---------------------------------------
}

void load_accept_file(char* acfile)
{
	acceptstate = new bool [state_num];
	for (int count_ac=0;count_ac<state_num;count_ac++)
		acceptstate[count_ac]=false;

	ifstream inacc;
	inacc.open(acfile);
	while(inacc)
	{
		int acstate;
		inacc>>acstate;
		acceptstate[acstate]=true;
	}
	inacc.close();
}
//--------------------------------------------------
//  Input layout
int* intputlayout(int* a, int chunk)
{
	int* b;
	b=new int [len];
	long bound=len/chunk;

	for(long i=0;i<bound;i++)
		for(int j=0;j<chunk;j++)
			b[i*chunk+j]=a[j*bound+i];
	delete []a;
	return b;
}

// This function is used for output layout
int* outputlayout(int* a)
{
	int* b;
	b=new int [len];
	long bound=len/spec_split1;
	long* c1=new long [spec_split1];
	for(int c2=0;c2<spec_split1;c2++)
		c1[c2]=c2*bound;

	for(int i=0;i<spec_split1;i++)
		for(long j=0;j<bound;j++)
			b[c1[i]+j]=a[j*spec_split1+i];

	delete []a;
	return b;
}
//--------------------------------------------------
//------------------Global instructions for pthread------------------------
void pthread_predict_complex(int chunk)	// This method use all state lookback
{
	int lookback_l;
	lookback_l=10000/state_num;	//use a lightweight lookback techniques
	int i,j,cpk;
	pthread_predict=new int [chunk];
	charlen=len/chunk;

	pthread_predict[0]=start_state;
	for (j=1;j<chunk;j++)
	{
		int look=len*j/chunk;
		int* predict_ss;
		int* timep;
		int ite=0;
		int maxt=0;

		predict_ss = new int [state_num];
		timep=new int [state_num];
		for (i=0;i<state_num;i++)
		{
			predict_ss[i]=i;
			timep[i]=0;
		}
		for(i=0;i<lookback_l;i++)
		{
			int symbol=input[look-lookback_l+i];
			for(cpk=0;cpk<state_num;cpk++)
				predict_ss[cpk]=T3[predict_ss[cpk]][symbol];
		}
		for(i=0;i<state_num;i++)
			timep[predict_ss[i]]++;
		for (i=0;i<state_num;i++)
		{	
			if(timep[i]>maxt)
			{
				maxt=timep[i];
				ite=i;
			}
		}	

		pthread_predict[j]=ite;
		delete []predict_ss;
		delete []timep;
	}
}

//-------------------------------------------------
void thread_Recompute_match (int chunk, int selec)
{
	rep = new long [chunk];
	WHOLEFINAL =  new int [chunk];		// The results stored in WHOLEFINAL must be true	
	for (int a1=0;a1<chunk;a1++)
	{
		rep[a1]=0;
		WHOLEFINAL[a1]=pthread_final[a1];
	}
	long i;
	for(int c1=0;c1<chunk-1;c1++)
	{
		if(WHOLEFINAL[c1] != pthread_predict[c1+1])
		{
			i=charlen*(c1+1);
			int temp1=WHOLEFINAL[c1];
			int temp2=pthread_predict[c1+1];
			do
			{
				if (selec==3)
				{
					temp1=T3[temp1][input[i]];
					temp2=T3[temp2][input[i]];
				}
				i++;
				if(temp1==temp2)
					break;
				if(acceptstate[temp1])
					match++;
				if(acceptstate[temp2])
					match--;
			}while(i<(charlen*(c1+2)));
			cout<<"The reprocessing length in thread "<<c1+1<<" is "<<(i-charlen*(c1+1))<<endl;
			rep[c1+1]=(i-charlen*(c1+1));
			if(i==charlen*(c1+2))
				WHOLEFINAL[c1+1]=temp1;
		}
	}
}

void reprocess_information()
{
	/* Getting the reprocess information */
	int wrong_pred_num = 0;
	double wrong_pred = 0.0;
	long totalmislen = 0;

	ofstream out_ana_rep;
	ofstream out_mispredict;
	ofstream out_mislen;
	
	out_ana_rep.open("reprocess_analyse_information.txt", ios::app);
	out_mispredict.open("miss_prediction_rate.txt", ios::app);
	out_mislen.open("miss_prediction_length.txt", ios::app);
	out_ana_rep<<spec_split1<<" "<<len/spec_split1<<" "<<(double)1.0/(spec_split1)<<endl;
	
	for (int a1=0;a1<spec_split1;a1++)
	{
		out_ana_rep<<pthread_predict[a1]<<" ";
		if (pthread_predict[a1]!=WHOLEFINAL[a1-1]&&a1>0)
			wrong_pred_num++;
	}
	out_ana_rep<<endl;
	wrong_pred=(double)wrong_pred_num/(double)spec_split1;
	out_mispredict<<spec_split1<<" "<<wrong_pred<<endl;
	out_mispredict.close();

	for (int a2=0;a2<spec_split1;a2++)
		out_ana_rep<<WHOLEFINAL[a2]<<" ";
	out_ana_rep<<endl;
	for (int a3=0;a3<spec_split1;a3++)
		out_ana_rep<<rep[a3]<<" ";
	out_ana_rep<<endl;
	double total_reprecess_perc=0.0;
	for (int a4=0;a4<spec_split1;a4++)
	{
		totalmislen += rep[a4];
		total_reprecess_perc+=(double)rep[a4]/len;
		out_ana_rep<<(double)rep[a4]/len<<" ";
	}
	out_ana_rep<<"------------------"<<endl<<endl;
	out_ana_rep.close();
	
	//-----------------------------------------------------
	ofstream out_re;
	out_re.open("total_reprocess_information.txt", ios::app);
	if(spec_split1==1)
	{
		out_mislen<<spec_split1<<" "<<0<<" "<<0.0<<endl;
		out_re<<spec_split1<<" "<<0.0<<endl;
	}
	else
	{
		out_mislen<<spec_split1<<" "<<totalmislen<<" "<<(double)totalmislen*1.0/(spec_split1-1)<<endl;
		out_re<<spec_split1<<" "<<total_reprecess_perc<<endl;
	}
	out_re.close();
	out_mislen.close();
}



bool* papi_test_n;
void papifill(char* tfilename)
{
	papi_test_n = new bool [256];
	int papitemp;
	for(papitemp=0;papitemp<256;papitemp++)
		papi_test_n[papitemp]=false;
	
	ifstream in_papi;
	in_papi.open(tfilename);
	while(in_papi)	
	{
		in_papi>>papitemp;
		papi_test_n[papitemp-1]=true;
	}
	in_papi.close();	
}

#endif
