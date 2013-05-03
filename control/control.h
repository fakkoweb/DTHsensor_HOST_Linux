#ifndef _CONTROL_H_
#define _CONTROL_H_
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

#include "hidapi.h"

using namespace std;




typedef struct _MEASURE_STRUCT
{
	unsigned short int dust;
	short int temp;			//unsigned??
	unsigned short int humid;
} measure_struct;

void p_sleep(unsigned milliseconds);


class control
{
	private:
		//Attributi della classe specifici per il controllo
		char new_terminal_path[13];
		ofstream* new_terminal_out;
		pid_t window;
		mutex rw;
		thread* c;
	
		//Flag per il controllo (protetti da mutex)
		bool stop;
		bool keep_console;
		int state;
		
		//Funzione per il controllo (thread di debug)
		void open_console();
		
		//Funzioni nascoste per usb
		int recv_measure(hid_device* d, measure_struct &m);
		
	public:
		
		//Membri della classe specifici per il controllo
		control();
		~control();
		
		void set_stop(bool value){ lock_guard<mutex> access(rw); stop=value; };
		bool get_stop(){ lock_guard<mutex> access(rw); return stop; };
		void close_console(){ lock_guard<mutex> access(rw); keep_console=false; };
		bool console_is_open(){ lock_guard<mutex> access(rw); return keep_console; };
		void set_state(int value){ lock_guard<mutex> access(rw); state=value; };
		int get_state(){ lock_guard<mutex> access(rw); return state; };

		//Funzioni del programma
		int scan();
		int read_show();
			
	
};



/////////////////
#endif

