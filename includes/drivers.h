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



/* COME REALIZZO UNA INTERFACCIA PER USB, RASPBERRY ed altri driver?? (la classe è statica non si può usare virtual!)
class Driver
{

    private:
        Driver();
            
    protected:
        static int request_delay;

    public:

        static short int request(){cout<<"Sono driver generico";};   //THIS FUNCTION MUST BE SPECIALIZED BY INHERITING CLASSES (error!!)
        static driver_call isr(){return NULL;};

};
//NON SI PUO' USARE PERCHE: non si può fare una interfaccia per le classi static, non si può fare static un singleton
*/



//CLASSE NON ISTANZIABILE
//Contiene tutte le funzioni relative a USB
class Usb
{
    private:
        Usb();
        static hid_device* d;                       //Selected hardware device via HID protocol
        static int request_delay = HARDWARE_DELAY;  
        static measure_struct m;                    //Contains last raw data extracted
        
        static int recv_measure();                  //Takes a new measure_struct from d via HID protocol from physical device
        
    public:
        static void init(){
            d=NULL;
            request_delay=HARDWARE_DELAY;
            m.temp=0;
            m.humid=0;
            m.dust=0;
        };
        static short int request(int type);         //TO IMPLEMENT!!! Calls recv_measure if request_delay has passed since last call
        static driver_call isr(){ return static_cast<driver_call>(request); };
    
    	//Funzioni generiche usb
		static int scan();
		
		
		//VECCHIE FUNZ.
		//static int recv_measure(hid_device* d, measure_struct &m);	//riceve una singola misura da device usb
		
		//Debug
		//static int read_show(const unsigned int times, const unsigned int delay);

    
};



//CLASSE NON INSTANZIABILE
//Contiene tutte le funzioni relative a RASP
class Raspberry
{
    private:
        Raspberry();
        static int request_delay;  
        static measure_struct m;                    //Contains last raw data extracted
        
        static int recv_measure();                  //TO IMPLEMENT!! Takes a new measure_struct from physical device
        
    public:
        static void init(){
            request_delay=HARDWARE_DELAY;
            m.temp=0;
            m.humid=0;
            m.dust=0;
        };
        static short int request(int type);         //TO IMPLEMENT!! Calls recv_measure if request_delay has passed since last call
        static driver_call isr(){ return static_cast<driver_call>(request); };      
        
        //Funzioni generiche raspberry
        
        
        //VECCHIE FUNZ.
    	//static int recv_measure(hid_device* d, measure_struct &m);	//riceve una singola misura da device usb

};


/////////////////
#endif

