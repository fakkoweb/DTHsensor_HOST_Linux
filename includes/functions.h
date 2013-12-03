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
#include "http_manager.h"


using namespace std;






//////////////////////////

// GENERIC USE FUNCTIONS
float faverage(float* array, int dim);
float fvariance(float* array, int dim);
string getTimeStamp();


/////////////////////////////////////////////////////////
// WRAP FOR CURL FOR http requests based on JSON STRINGS
//size_t write_callback(void *ptr, size_t size, size_t nmemb, void *stream);
//int http_get(const string url, string& json);
//int http_post(const string url, const string json);


////////////////////////////////////////
// PARAMETERS LOADING FROM A json file
// LOADS PARAMETERS TO GLOBALS IN config.h
// OLD!! int param_load(param_struct& user_config,const string filename);
Json::Value load_params(const string jsonfile);

//////////////////////
// PROJECT ROUTINES - ALL RETURN ERROR,ABORTED,NICE
int register_device(const string mac_addr);                      	//Checks if MAC_ADDRESS is registered. If not, registers. RETURNS the deviceID.
int register_sensors(const string mac_addr, const Json::Value& sd);  	//Checks if Sensor is registered. If not, registers. RETURNS ERROR,ABORT,NICE
									//The Json::Value is for registering further data like "tags"
int register_sensor(const string mac_addr, const Json::Value& node, const string rs);	//The recursive call from register_sensors (to check all json nodes recursively)									
int post_report(const string mac_addr, const map<int, Sensor*>& sa);   	//Gets statistics from Sensors (without waiting!!) in sa and posts it to server





///////////////////////
#endif










