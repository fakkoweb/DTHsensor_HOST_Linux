#ifndef _SENSORS_H_
#define _SENSORS_H_
////////////////////


//Include user configuration
#include "config.h"

//Internal dependencies
#include "drivers.h"

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
	#include <condition_variable>
#endif

//External libs
#include "p_sleep.h"


using namespace std;



class Sensor                                        //ABSTRACT CLASS: only sub-classes can be instantiated!
{
    protected:
    
        //BUFFERING STRUCTURES
        short int *raw_buffer;
        float *format_buffer;
        int next;
        int buffer_lenght;                          //IMPOSTATO A SECONDA DEL TIPO DI SENSORE!! -- da parameters.json
        bool buffer_filled;			//when do we reach the end of buffer?
        float raw_average;
        float average;
        float raw_variance;
        float variance;
        
        //BUFFERING LOW LEVEL OPERATIONS (NOT SAFE!! use locks before calling them!!)
        void push(const short int elem)
        {
            raw_buffer[next]=elem;
            format_buffer[next]=convert(elem);
            next=(next+1)%buffer_lenght;
            if(next==0) buffer_filled=true;
            else buffer_filled=false;
        };
        short int raw_top()
        {
        	int elem=next-1;
        	if(elem>=0) return raw_buffer[elem];
        	else return raw_buffer[buffer_lenght-1];
        };
        float format_top()
        {
        	return convert(raw_top());		//faster
        };
        short int raw_pick(const int index)
        { 
            int location;
            if(index<buffer_lenght && index>=0)
            {
                if(index>next) location=buffer_lenght-(index-next);
                else location=(next-index)%buffer_lenght;
                
                if (location > 0) return raw_buffer[location-1];
                else return raw_buffer[buffer_lenght-1];
            }
            else return 0;
        };
        float format_pick(const int index)
        {
        	return convert(raw_pick(index));	//faster
        };
        
        //SAMPLING & CONVERSION
        virtual short int sample() = 0;             //Chiamata da get_measure, semplicemente chiama board (la request() del driver associato)
        virtual float convert(const short int) = 0;       //THIS FUNCTIONS MUST BE SPECIALIZED BY INHERITING CLASSES
        
        //SENSOR CONTROL
        Driver<measure_struct,short int>* board;//Puntatore all'oggetto Driver da cui chiamare la funzione request() per chiedere il campione
        bool autorefresh;                       //TRUE: pooling attivo, FALSE: campionamento solo su richiesta (get_measure)        
                                                //Se autorefresh è TRUE: ogni quanto viene fatta richiesta di una nuova misura al driver (sample)
                                                //Se autorefresh è FALSE, è il tempo minimo tra una richiesta manuale e un'altra.
        int refresh_rate;                       //IMPOSTATO A SECONDA DEL TIPO DI SENSORE!! UTILE SOLO SE autorefresh E' TRUE -- in secondi
        void refresh();                         //Questa funzione chiama sample() e convert() e inserisce nuove misure nei buffer
                                                //Se autorefresh è TRUE viene chiamata da un thread ogni min_sample_rate oppure manualmente da get_measure
                                                //Se autorefresh è FALSE solo get_measure può chiamarla
        void reset();
                                                
        //THREADING STRUCTURES
        mutex rw;
        condition_variable new_sample;
        condition_variable new_statistic;
        thread* r;
        bool close_thread;
        
        
    public:
    	//"safe" si intende nel contesto in cui UN SOLO thread usi la classe e sia attivo il thread per l'autosampling
    	
    	//COSTRUTTORE & DISTRUTTORE
        Sensor() = delete;                          //disabling zero-argument constructor completely
        explicit Sensor(const int sample_rate, const int avg_interval, const bool enable_autorefresh = true); //sample_rate = secondi per l'autocampionamento (se attivato)
                                                                                            //avg_interval = minuti ogni quanto viene calcolata la media
        ~Sensor();	//safe
        
        //METODI DI ACCESSO PRIMARI (gestiscono i lock)
        short int get_raw(const int index=0);	//safe                          //Restituisce l'ultima misura. Se autorefresh è FALSE ed è trascorso min_sample_rate
                                                                        //dall'ultima chiamata, richiede anche una nuova misura (sample), altrimenti da l'ULTIMA effettuata
                                                                        //( in futuro: IMPLEMENTARE una versione che dia il measure_code della misura restituita )
        float get_raw_average(){ lock_guard<mutex> access(rw); return raw_average; };	//safe
        float get_raw_variance(){ lock_guard<mutex> access(rw); return raw_variance; };	//safe
        void wait_new_sample(); //safe                                  //Se chiamata, ritorna solo quando il sensore effettua la prossima misura
                                                                        //- HA EFFETTO SOLO SE AUTOREFRESH E' ATTIVO (altrimenti non ha senso perchè la richiesta la farebbe get_measure)
                                                                        //- E' UTILE SE SUBITO DOPO VIENE CHIAMATA get_measure
        void wait_new_statistic(); //safe                               //Stessa cosa di wait_new_sample() ma per media e varianza        
        
        //METODI SECONDARI (sfruttano i metodi primari)
        float get_measure(const int index=0){ return convert(get_raw(index)); };//Stessa cosa di get_raw() ma la converte prima di restituirla                        
        float get_average(){ return convert(get_raw_average()); };
        float get_variance(){ return convert(get_raw_variance()); };
        void display_measure(){ cout<<get_measure()<<endl; };
        void plug_to(const Driver<measure_struct,short int>& new_board);//Associa un driver (e la sua request()) al sensore virtuale da chiamare a ogni sample() --> //safe

};

class TempSensor : public Sensor
{
    protected:
        virtual short int sample(){ return board->request(TEMPERATURE); };      //Chiamata da get_measure, semplicemente chiama request() di board.    
        virtual float convert(const short int);                                       //TO IMPLEMENT!!
    public:
        TempSensor(const int refresh_rate, const int avg_interval, const bool enable_autorefresh = true) : Sensor(refresh_rate,avg_interval,enable_autorefresh) {};
};

class HumidSensor : public Sensor
{
    protected:
        virtual float convert(const short int);                                       //TO IMPLEMENT!!
        virtual short int sample(){ return board->request(HUMIDITY); };         //Chiamata da get_measure, semplicemente chiama request() di board.           
    public:
        HumidSensor(const int refresh_rate, const int avg_interval, const bool enable_autorefresh = true) : Sensor(refresh_rate,avg_interval,enable_autorefresh) {};
};

class DustSensor : public Sensor
{
    protected:
        virtual float convert(const short int);                                       //TO IMPLEMENT!!
        virtual short int sample(){ return board->request(DUST); };             //Chiamata da get_measure, semplicemente chiama request() di board.           
    public:
        DustSensor(const int refresh_rate, const int avg_interval, const bool enable_autorefresh = true) : Sensor(refresh_rate,avg_interval,enable_autorefresh) {};
};



/////////////////
#endif

