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



typedef short int (*driver_call)();


typedef struct _MEASURE_STRUCT
{
	short int dust;
	short int temp;			//unsigned??
	short int humid;
} measure_struct;

void p_sleep(unsigned milliseconds);



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



//CLASSE NON ISTANZIABILE
//Contiene tutte le funzioni relative a USB
class Usb : public Driver
{
    private:
        Usb();
    
    protected:
        static int request_delay;
        static measure_struct external;    //last raw data extracted
        
    public:
        static short int request(){cout<<"Sono usb";};
        static driver_call isr(){return static_cast<driver_call>(request);};
    
    	//Funzioni generiche usb
		//static int scan();
		
		
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
    
    protected:
        static int request_delay;
        static measure_struct internal;    //last raw data extracted
        
    public:
        static short int request(){cout<<"Sono raspberry";};
        static driver_call isr(){return static_cast<driver_call>(request);};      
        //Funzioni generiche raspberry
        
        
        //VECCHIE FUNZ.
    	//static int recv_measure(hid_device* d, measure_struct &m);	//riceve una singola misura da device usb

};



class Sensor                //ABSTRACT CLASS: only sub-classes can be instantiated!
{
    protected:
        int type;                         //Distingue il tipo di sensore (serve alla recv_measure)
        short int raw_buffer[SENSOR_BUFFER];
        float format_buffer[SENSOR_BUFFER];
        int refresh_rate;                 //Ogni quanto viene effettivamente richiesta una nuova misura
                                                //Se il pooling è attivo, è automatico, altrimenti è il tempo minimo
                                                //tra una richiesta manuale e un'altra.
        int last_measure_code;                  //Restituito assieme alla misura, serve a chi la richiede per capire
                                                //se è effettivamente nuova oppure no.
        driver_call board;
    public:        
        virtual float convert(short int) = 0;   //THIS FUNCTION MUST BE SPECIALIZED BY INHERITING CLASSES
        //virtual float sample();                 //Chiamata da get_measure, semplicemente chiama request() di board.


        Sensor(){refresh_rate=SENSOR_REFRESH_RATE;type=UNDEF;};
        //float get_measure();                    
        //float get_measure(int index);
        //void refresh();     //pushes a new sample in raw_buffer and converts it in format_buffer
                            //can be called manually or periodically by a thread!
        void plug_to(const driver_call new_board){board=new_board;};
    
};

class TempSensor : public Sensor
{

    public:
        TempSensor(){type=TEMP;};
    //protected:
        virtual float convert(short int){(*board)();};        
};

class HumidSensor : public Sensor
{
    protected:
        virtual float convert(short int){cout<<"convertita";};
    public:
        HumidSensor(){type=HUMID;};
};

class DustSensor : public Sensor
{
    protected:
        virtual float convert(short int){cout<<"convertita";};
    public:
        DustSensor(){type=DUST;};
};
/*

// DA ISTANZIARE UNA VOLTA ALL'INIZIO DEL MAIN
//Inizializza e dealloca tutte le variabili dell'ambiente e il debug
class control
{
	private:
		//CONTROLLO CONSOLE
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
		
		
	public:
		
		control();
		~control();
		
		//Membri della classe specifici per il controllo
		void set_stop(bool value){ lock_guard<mutex> access(rw); stop=value; };
		bool get_stop(){ lock_guard<mutex> access(rw); return stop; };
		void close_console(){ lock_guard<mutex> access(rw); keep_console=false; };
		bool console_is_open(){ lock_guard<mutex> access(rw); return keep_console; };
		void set_state(int value){ lock_guard<mutex> access(rw); state=value; };
		int get_state(){ lock_guard<mutex> access(rw); int old_state=state; state=false; return old_state; };


};

*/

/////////////////
#endif

