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


//DEPRECATA: Driver call usata quando i driver erano statici -> puntatore a funzione
//typedef short int (*driver_call)();

typedef struct _MEASURE_STRUCT
{
	short int dust;
	short int temp;
	short int humid;
} measure_struct;


//CLASSE NON ISTANZIABILE -- Rappresenta una device generica che su richiesta restituisce una struct "data_type" contenente diversi "elem_type"
template <class data_type, class elem_type>
class Driver
{
 
    protected:
        std::chrono::duration< int, std::milli > request_delay;		//GESTIONE DELAY HARDWARE: Se più sensori/processi/thread fanno richiesta al driver di seguito,
        std::chrono::steady_clock::time_point last_request;		//limito le richieste effettive inoltrate all'hardware fisico (tengo conto dei ritardi intrinseci e
        								//li nascondo al programmatore) SOPRATTUTTO le richieste INUTILI (ad esempio, in caso di errore,
        								//mi basta che sia la prima richiesta a segnalarlo per le richieste subito successive).
        
        virtual bool ready();               //TESTS IF DEVICE IS READY (NOT BUSY!) TO SATISFY A NEW REQUEST
        				    //Implements generic driver algorithms to counter hardware limits:
        				    //	- Minimum delay between requests
        				    //CAN BE EXTENDED BY DERIVED CLASSES!!
        
        data_type m;                        //Contains last raw data extracted by recv_measure
        virtual int recv_measure()=0;       //Takes a new data_type from d via HID protocol from physical device
                                            //RETURNS error code.

    public:
        Driver(const int min_delay = HARDWARE_DELAY){
            if(min_delay<=0) request_delay = std::chrono::duration< int, std::milli >::zero();
            else request_delay = std::chrono::milliseconds(min_delay);
            last_request = std::chrono::steady_clock::now() - request_delay;//This way first recv_measure is always done!!
        };
        data_type request_all(){ if(ready()) recv_measure(); return m; };   //A default function that returns the WHOLE data_type
                                                                            //NOTICE: testing "ready()" assures you respect device timing!
        virtual elem_type request(const int type)=0;	//RETURN an element from data_type. A new recv_measure() should be called testing
        					//the ready() condition.

};



//CLASSE ISTANZIABILE -- Rappresenta una device USB via HID
//Contiene tutte le funzioni relative a USB
class Usb : public Driver<measure_struct,short int>
{
    protected:
        hid_device* d;                      //Selected hardware device via HID protocol. IF IT IS NULL, NO DEVICE IS CONNECTED SO CHECK FIRST!!
                                            //Device is "opened" at first call of recv_measure()
        int vid;
        int pid;
        virtual bool ready();               //TESTS IF DEVICE IS READY (NOT BUSY!) TO SATISFY A NEW REQUEST
        				    //Implements usb driver algorithms to counter hardware limits:
        				    //	(base) Minimum delay between requests
        				    //	(extension) Device IS plugged in (issue a scan if not)
        				    //CAN BE EXTENDED BY DERIVED CLASSES!!	
        virtual int recv_measure();         //SPECIALIZED: Takes a new "measure_struct" from d via HID protocol from physical device.
                                            //RETURNS error code.
        
    public:
        Usb() = delete; 
        Usb(const int vid_in, const int pid_in, const int min_delay = HARDWARE_DELAY) : Driver(min_delay){
            d=NULL;
            m.temp=0;
            m.humid=0;
            m.dust=0;
            vid=vid_in;
            pid=pid_in;
        };
        ~Usb()
        {
            if(d!=NULL) hid_close(d);
            cout<<"  | Device chiusa."<<endl;
        };
        
        virtual short int request(const int type);        //SPECIALIZED: Calls recv_measure if request_delay has passed since last call
                                                    //RETURNS measure of type selected from m

    
    	//Funzioni generiche (static) usb
	static int scan(const int vid, const int pid);
		
		
		//VECCHIE FUNZ.
		//static int recv_measure(hid_device* d, measure_struct &m);	//riceve una singola misura da device usb
		
		//Debug
		//static int read_show(const unsigned int times, const unsigned int delay);

    
};



//CLASSE INSTANZIABILE -- Rappresenta l'interfaccia seriale del Raspberry
//Contiene tutte le funzioni relative a RASP
class Raspberry : public Driver<measure_struct,short int>
{
    private:
        virtual int recv_measure();         //SPECIALIZED: TO IMPLEMENT!! Takes a new measure_struct from physical device
        
    public:
        Raspberry(const int min_delay = HARDWARE_DELAY) : Driver(min_delay){
            m.temp=0;
            m.humid=0;
            m.dust=0;
        };
        
        virtual short int request(const int type);        //Calls recv_measure if request_delay has passed since last call
                                            //RETURNS measure of type selected from m    
        
        //Funzioni generiche raspberry
        
        
        //VECCHIE FUNZ.
    	//static int recv_measure(hid_device* d, measure_struct &m);	//riceve una singola misura da device usb

};


/////////////////
#endif

