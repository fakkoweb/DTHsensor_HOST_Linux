#ifndef _DRIVERS_H_
#define _DRIVERS_H_
////////////////////


//Include user configuration
#include "config.h"

//Standard included libraries
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <chrono>
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
#include "hidapi.h"
#include "p_sleep.h"


using namespace std;



typedef short int (*driver_call)();

typedef struct _MEASURE_STRUCT
{
	short int dust;
	short int temp;
	short int humid;
} measure_struct;


template <class data_type>
class Driver
{
 
    protected:
        std::chrono::duration<int> request_delay;
        std::chrono::steady_clock::time_point last_request;
        bool ready();                       //Tests if device is ready to do a new request!
        
        data_type m;                        //Contains last raw data extracted by recv_measure
        virtual int recv_measure()=0;       //Takes a new data_type from d via HID protocol from physical device
                                            //RETURNS error code.

    public:
        Driver(int min_delay = HARDWARE_DELAY){
            if(min_delay<=0) request_delay = std::chrono::duration::zero;
            else request_delay = std::chrono::milliseconds(min_delay);
            last_request = std::chrono::steady_clock::now() - request_delay;//This way first recv_measure is always done!!
        };
        data_type request_all(){ if(ready()) recv_measure(); return m; };   //A default function that returns the WHOLE data_type
                                                                            //NOTICE: testing "ready()" assures you respect device timing!

};



//CLASSE NON ISTANZIABILE
//Contiene tutte le funzioni relative a USB
class Usb : public Driver<measure_struct>
{
    protected:
        hid_device* d;                      //Selected hardware device via HID protocol
        virtual int recv_measure();         //SPECIALIZED: Takes a new "measure_struct" from d via HID protocol from physical device.
                                            //RETURNS error code.
        
    public:
        Usb(int min_delay = HARDWARE_DELAY) : Driver(min_delay){
            d=NULL;
            m.temp=0;
            m.humid=0;
            m.dust=0;
        };
        short int request(int type);        //Calls recv_measure if request_delay has passed since last call
                                            //RETURNS measure of type selected from m

    
    	//Funzioni generiche usb
		int scan();
		
		
		//VECCHIE FUNZ.
		//static int recv_measure(hid_device* d, measure_struct &m);	//riceve una singola misura da device usb
		
		//Debug
		//static int read_show(const unsigned int times, const unsigned int delay);

    
};



//CLASSE NON INSTANZIABILE
//Contiene tutte le funzioni relative a RASP
class Raspberry : public Driver<measure_struct>
{
    private:
        virtual int recv_measure();         //SPECIALIZED: TO IMPLEMENT!! Takes a new measure_struct from physical device
        
    public:
        Raspberry(int min_delay = HARDWARE_DELAY) : Driver(min_delay){
            m.temp=0;
            m.humid=0;
            m.dust=0;
        };
        short int request(int type);        //Calls recv_measure if request_delay has passed since last call
                                            //RETURNS measure of type selected from m    
        
        //Funzioni generiche raspberry
        
        
        //VECCHIE FUNZ.
    	//static int recv_measure(hid_device* d, measure_struct &m);	//riceve una singola misura da device usb

};


/////////////////
#endif

