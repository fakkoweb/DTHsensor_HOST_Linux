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
#include <map>
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
#include <json.h>	//for accessing Json::Value

//Internal libs
#include "sensors.h"


using namespace std;






//////////////////////////
// GENERIC MATH FUNCTIONS
double faverage(double* array, int dim);
double fvariance(double* array, int dim);


/////////////////////////////////////////////////////////
// WRAP FOR CURL FOR http requests based on JSON STRINGS
size_t write_callback(void *ptr, size_t size, size_t nmemb, void *stream);
int http_get(const string url, string& json);
int http_post(const string url, const string json);


////////////////////////////////////////
// PARAMETERS LOADING FROM A json file
// LOADS PARAMETERS TO GLOBALS IN config.h
// OLD!! int param_load(param_struct& user_config,const string filename);


//////////////////////
// PROJECT ROUTINES
Json::Value load_params(const string jsonfile);
int register_device(const int vid, const int pid);                      //Checks if MAC_ADDRESS is registered. If not, registers. RETURNS the deviceID.
int register_sensors(const int device_id, const map<int, Sensor*>& sa); //Checks if Sensor is registered. If not, registers. RETURNS ERROR,ABORT,NICE
												//The Json::Value is for registering further data like "tags"
int post_report(const map<int, Sensor*>& sa);   			//Gets statistics from Sensors (without waiting!!) in sa and posts it to server
                                                                        //RETURNS ERROR,ABORT,NICE




///////////////////////
#endif










