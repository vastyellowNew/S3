#ifndef __PapiInitialization_H__
#define __PapiInitialization_H__

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
#include "papi.h"

#define MAX_EVENTS 256

using namespace std;  

int PAPI_num_counters(void);
int retval;				// Test whether there is anything wrong
int nun_component;		// The number of component
int component_id;		// The id of component
static int num_events;
int EventSet;
int rapl_component_id;
const PAPI_component_info_t *cmpinfo;
int enum_retval;
char events[MAX_EVENTS][BUFSIZ];
char filenames[MAX_EVENTS][BUFSIZ];
char event_name[BUFSIZ];
long long values[MAX_EVENTS];
long long pstart_time,pbefore_time,pafter_time;
double pelapsed_time,ptotal_time;
int papithread;

bool Papi_Initialization()
{
	num_events=0;
	rapl_component_id=-1;
	EventSet= PAPI_NULL;
	cmpinfo=NULL;
	int code;

	retval = PAPI_library_init( PAPI_VER_CURRENT );
	nun_component = PAPI_num_components();
	for(component_id=0; component_id<nun_component; component_id++) 
	{
	  	if ( (cmpinfo = PAPI_get_component_info(component_id)) == NULL) 
	    {
	       	cout<<"PAPI_get_component_info failed"<<endl;
	       	return true;
	    }
	    if (strstr(cmpinfo->name,"rapl")) 
	    {
	       	rapl_component_id=component_id;
	       	cout<<"Found rapl component at component_id "<<rapl_component_id<<endl;

	        if(cmpinfo->disabled) 
	        {
	        	cout<<"No rapl events found "<<cmpinfo->disabled_reason<<endl;
	       		return true;
	       	}
		   	break;
		}	
	}

	/* Component not found */
	if (component_id==nun_component) 
	{
  		cout<<"No rapl component found"<<endl;
       	return true;
   	}
    	
    /* Find Events */
    code = PAPI_NATIVE_MASK;
    enum_retval = PAPI_enum_cmp_event( &code, PAPI_ENUM_FIRST, component_id );
    while ( enum_retval == PAPI_OK ) 
	{
	    retval = PAPI_event_code_to_name( code, event_name );
		cout<<"Found "<<event_name<<endl;
	    strncpy(events[num_events],event_name,BUFSIZ);
	    sprintf(filenames[num_events],"results.%s",event_name);
	    num_events++;
	    enum_retval = PAPI_enum_cmp_event( &code, PAPI_ENUM_EVENTS, component_id );
	}
	/* Create EventSet */
	retval = PAPI_create_eventset( &EventSet );
	for(int i=0;i<num_events;i++) 
	   	retval = PAPI_add_named_event( EventSet, events[i]);

	return false;
}


int record_cp;
double record_power[6];
double record_energy[6];
double total_power_sum;
double total_energy_sum;
ofstream outenergy;
ofstream out_avpower;
ofstream total_energy;

void Papicalculation()
{
	out_avpower.open("Total_average_power.txt",ios::app);
	outenergy.open("Detailed_outenergy.txt",ios::app);
	total_energy.open("Total_running_energy.txt",ios::app);

	record_cp=0;
	total_power_sum=0.0;
	total_energy_sum=0.0;
	for(int i=0;i<num_events;i++) 
    {	
		if(strstr(events[i],"PACKAGE_ENERGY:PACKAGE")||
		   		strstr(events[i],"DRAM_ENERGY:PACKAGE")||
		   		strstr(events[i],"PP0_ENERGY:PACKAGE"))
		{
			outenergy<<mode<<" "<<papithread<<endl;
	       	outenergy<<((double)values[i]/1.0e9)<<" ";
			outenergy<<((double)values[i]/1.0e9)/pelapsed_time;
	       	outenergy<<" "<<"(* Average Power for "<<events[i]<<" *)";
			outenergy<<endl<<endl;

			record_power[record_cp]=((double)values[i]/1.0e9)/pelapsed_time;
			record_energy[record_cp]=(double)values[i]/1.0e9;
			record_cp++;
		}
	}
	total_energy_sum=record_energy[0]+record_energy[1]+record_energy[2]+record_energy[3];
	total_power_sum=record_power[0]+record_power[1]+record_power[2]+record_power[3];
	out_avpower<<papithread<<" "<<total_power_sum<<endl;
	total_energy<<papithread<<" "<<total_energy_sum<<endl;
	out_avpower.close();
	outenergy.close();
	total_energy.close();
}
#endif
