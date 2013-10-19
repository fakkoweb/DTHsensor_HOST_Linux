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
int param_load(const string filename);


//////////////////////
// PROJECT ROUTINES

int register_device();                          //Checks if MAC_ADDRESS is registered. If not, registers. RETURNS the deviceID.
int register_sensor(Sensor*,int device_id);     //Checks if Sensor is registered. If not, registers. RETURNS the local_feed_ID.
void report_routine();















