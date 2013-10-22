#ifndef _FUNCTIONS_H_
#define _FUNCTIONS_H_
////////////////////


//Include user configuration
#include "config.h"

//Standard included libraries
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/ipc.h>
    #include <sys/types.h>
    #include <sys/wait.h>
    #include <sys/shm.h>
    #include <unistd.h>
    #include <string.h>
#endif
#ifdef HOST_THREADS
	#include <mutex>
	#include <thread>
#endif

//External libs
#include "p_sleep.h"


using namespace std;




//Struct used to load parameters from json file
typedef struct _PARAM_STRUCT
{
    int MY_VID ;
    int MY_PID ;
    int EXT_TEMP_lfid ;
    int EXT_HUMID_lfid ;
    int EXT_DUST_lfid ;
    int INT_TEMP_lfid ;
    int INT_HUMID_lfid ;
    int TEMP_REFRESH_RATE ;
    int HUMID_REFRESH_RATE ;
    int DUST_REFRESH_RATE ;
    int REPORT_INTERVAL ;
} param_struct;




//////////////////////////
// GENERIC MATH FUNCTIONS
float average(float* array, int dim);
float variance(float* array, int dim);


/////////////////////////////////////////////////////////
// WRAP FOR CURL FOR http requests based on JSON STRINGS
size_t write_callback(void *ptr, size_t size, size_t nmemb, void *stream);
int http_get(const string url, string& json);
int http_post(const string url, const string json);


////////////////////////////////////////
// PARAMETERS LOADING FROM A json file
// LOADS PARAMETERS TO GLOBALS IN config.h
int param_load(param_struct& user_config,const string filename);


//////////////////////
// PROJECT ROUTINES
int register_device(const int vid, const int pid);                      //Checks if MAC_ADDRESS is registered. If not, registers. RETURNS the deviceID.
int register_sensors(const int device_id, map<int, Sensor*>& sa);       //Checks if Sensor is registered. If not, registers. RETURNS ERROR,ABORT,NICE
int report_routine(const int device_id, const map<int, Sensor*>& sa);   //Waits for new measures/statistics from Sensors in sa and posts it to server
                                                                        //RETURNS ERROR,ABORT,NICE




///////////////////////
#endif










