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
        static short int request(int type);         //TO IMPLEMENT TIMER!! Calls recv_measure if request_delay has passed since last call
        static driver_call isr(){ return static_cast<driver_call>(request); };      
        
        //Funzioni generiche raspberry
        
        
        //VECCHIE FUNZ.
    	//static int recv_measure(hid_device* d, measure_struct &m);	//riceve una singola misura da device usb

};



class Sensor                //ABSTRACT CLASS: only sub-classes can be instantiated!
{
    protected:
    
        //BUFFERING STRUCTURES
        short int raw_buffer[SENSOR_BUFFER];
        int last_raw_index;
        float format_buffer[SENSOR_BUFFER];
        int last_format_intex;
        int last_measure_code;                  //Restituito assieme alla misura, serve a chi la richiede per capire
                                                //se è effettivamente nuova oppure no.
        void raw_push(short int elem){ last_raw_index=(last_raw_index+1)%SENSOR_BUFFER; raw_buffer[last_raw_index]=elem; };
        void format_push(float elem){ last_format_index=(last_format_index+1)%SENSOR_BUFFER; format_buffer[last_format_index]=elem; };
        short int raw_top(){ return raw_buffer[last_raw_index]; };
        float format_top(){ return format_buffer[last_format_index]; };
        
        //SAMPLING & CONVERSION
        virtual void sample() = 0;              //Chiamata da get_measure, semplicemente chiama board (la request() del driver associato)
        virtual float convert(short int) = 0;   //THIS FUNCTION MUST BE SPECIALIZED BY INHERITING CLASSES
        
        //SENSOR CONTROL
        driver_call board;                      //Puntatore restituito da isr() alla giusta funzione request() per chiedere il campione
        bool autorefresh;                       //TRUE: pooling attivo, FALSE: campionamento solo su richiesta (get_measure)        
        int min_sample_rate;                    //Se autorefresh è TRUE: ogni quanto viene fatta richiesta di una nuova misura al driver (sample)
                                                //Se autorefresh è FALSE, è il tempo minimo tra una richiesta manuale e un'altra.
        void refresh();                         //IMPLEMENTARE!! Questa funzione chiama sample() e convert() e inserisce nuove misure nei buffer
                                                //Se autorefresh è TRUE viene chiamata da un thread ogni min_sample_rate oppure manualmente da get_measure
                                                //Se autorefresh è FALSE solo get_measure può chiamarla
                                                //IMPLEMENTARE IL CONTROLLO TIMER
        
    public:  
        Sensor():raw_buffer{0},format_buffer{0}{refresh_rate=SENSOR_REFRESH_RATE;last_measure_code=0;board=NULL;last_raw_index=0;last_format_index=0;autorefresh=false;};
        Sensor(bool enable_autorefresh):raw_buffer{0},format_buffer{0}{refresh_rate=SENSOR_REFRESH_RATE;last_measure_code=0;board=NULL;last_raw_index=0;last_format_index=0;autorefresh=enable_autorefresh;};
        float get_measure(int index=0);                                 //Restituisce l'ultima misura. Se autorefresh è FALSE ed è trascorso min_sample_rate
                                                                        //dall'ultima chiamata, richiede anche una nuova misura (sample), altrimenti da l'ULTIMA effettuata
                                                                        //IMPLEMENTARE una versione che dia il measure_code della misura restituita!!
        void display_measure(){cout<<get_measure()<<endl;};
        void plug_to(const driver_call new_board){ if(board!=NULL) board=new_board; };  //Associa un driver (request()) al sensore virtuale da chiamare a ogni sample()
    
};

class TempSensor : public Sensor
{

    public:
        //TempSensor(){};
    protected:
        virtual short int sample(){ return (*board)(TEMPERATURE); };    //Chiamata da get_measure, semplicemente chiama request() di board.    
        virtual float convert(short int);                               //TO IMPLEMENT!!
};

class HumidSensor : public Sensor
{
    protected:
        virtual float convert(short int);                               //TO IMPLEMENT!!
        virtual short int sample(){ return (*board)(HUMIDITY); };       //Chiamata da get_measure, semplicemente chiama request() di board.           
    public:
        HumidSensor(){};
};

class DustSensor : public Sensor
{
    protected:
        virtual float convert(short int);                               //TO IMPLEMENT!!
        virtual short int sample(){ return (*board)(DUST); };           //Chiamata da get_measure, semplicemente chiama request() di board.           
    public:
        DustSensor(){};
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

