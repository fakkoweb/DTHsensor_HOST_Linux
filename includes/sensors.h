#ifndef _SENSORS_H_
#define _SENSORS_H_
////////////////////


//Include user configuration
#include "config.h"

//Internal dependencies
#include "drivers.h"
#include "OMV.h"

//Standard included libraries
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <sstream>
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

typedef struct _STATISTIC_STRUCT
{
	double average;
	double variance;
	bool valid;
	double percentage_validity;
	int tot_sample;
} statistic_struct;


class Sensor					//ABSTRACT CLASS: only sub-classes can be instantiated!
{
    protected:
    
        //BUFFERING VARIABLES
        uint16_t raw_measure;
        //OLD:	double format_measure;		//Memorizza una versione convertita di raw_buffer - non più necessaria, ora la misura è convertita su richiesta
        statistic_struct statistic;
        
        
        //AVERAGING AND VARIANCE CALCULATION	//asdfg
        OMV MeanGuy;							//Classe per il calcolo della media on-line (Knuth/Welford algorithm) --> vedi lista di inizializzazione del costruttore!
        std::chrono::duration< int, std::milli > avg_delay;		//GESTIONE AVERAGE INTERVAL: Ogni sample() del sensore viene calcolata una nuova media e varianza,
        std::chrono::steady_clock::time_point last_avg_request;		//basate sul valore corrente e sulla storia precedente. Ad ogni avg_delay (scelto in base alle esigenze)
        								//il sensore salva la media e resetta il calcolo di media e varianza, confrontando avg_delay con
        								//il tempo trascorso da last_avg_request. Poi setta last_avg_request al tempo corrente.
        
        
        //SAMPLING & CONVERSION
        virtual int mtype()=0;					//returns the type (its code) of measure sensor requests to driver
        uint16_t sample(){ return board->request(mtype()); };	//Chiamata da get_measure, semplicemente chiama board (la request() col tipo misura richiesto)
        virtual double convert(const uint16_t) = 0;       	//THIS FUNCTIONS MUST BE SPECIALIZED BY INHERITING CLASSES

        
        //SENSOR CONTROL
        Driver<measure_struct,uint16_t>* board;	//Puntatore all'oggetto Driver da cui chiamare la funzione request() per chiedere il campione
        int refresh_rate;                       //IMPOSTATO A SECONDA DEL TIPO DI SENSORE!! Inizializzato da costruttore a "sample_rate" ma è UTILE SOLO SE autorefresh E' TRUE -- in millisecondi
        bool autorefresh;                       //TRUE: pooling attivo, FALSE: campionamento solo su richiesta (get_measure)        
                                                //Se autorefresh è TRUE: ogni "refresh_rate" ms viene richiesta una nuova misura al driver (sample)
                                                //Se autorefresh è FALSE, "refresh_rate" è ignorato.
        void refresh();                         //Questa funzione chiama sample() e convert() e aggiorna il buffer raw_measure (e statistic solo se avg_interval è trascorso)
                                                //Se autorefresh è TRUE viene chiamata da un thread ogni "refresh_rate" ms oppure manualmente da get_raw()
                                                //Se autorefresh è FALSE solo get_raw() può chiamarla
        void reset();				//Resetta i valori del sensore a default (chiamata quando ad es. si vuole scollegare il sensore senza deallocarlo!)
                  
        virtual void set_offset(){};		//Abilita la correzione dell'offset per il sensore                                        
        //THREADING STRUCTURES
        mutex rw;				//Guarantees mutual access between autorefresh thread and external requesting threads
        condition_variable new_sample;
        condition_variable new_statistic;
        thread* r;
        bool close_thread;
        
        
    public:
    	//"safe" si intende nel contesto in cui UN SOLO thread usi la classe e sia attivo il thread per l'autosampling
    	
    	
    	//COSTRUTTORE & DISTRUTTORE
        Sensor() = delete;                          //disabling zero-argument constructor completely
        explicit Sensor(const int sample_rate, const int avg_interval, const bool enable_autorefresh = true);	//sample_rate = millisecondi per l'autocampionamento (se attivato)
                                                                                            			//avg_interval = minuti ogni quanto viene resettata la media (è calcolata on-line)
														//inizializzato: MeanGuy() = sceglie se considerare o meno un MinOffset per il calcolo della media
        ~Sensor();	//safe
        
        
        //METODI DI ACCESSO PRIMARI (gestiscono i lock)
        uint16_t get_raw();	//safe                          	//Restituisce l'ultima misura. Se autorefresh è FALSE ed è trascorso min_sample_rate
                                                                        //dall'ultima chiamata, richiede anche una nuova misura (sample), altrimenti da l'ULTIMA effettuata
                                                                        //( in futuro: IMPLEMENTARE una versione che dia il measure_code della misura restituita )

        /**double get_average(){ lock_guard<mutex> access(rw); return average; };
        double get_variance(){ lock_guard<mutex> access(rw); return variance; };**/
        statistic_struct get_statistic(){ lock_guard<mutex> access(rw); return statistic; };

        void wait_new_sample(); //safe                                  //Se chiamata, ritorna solo quando il sensore effettua la prossima misura
                                                                        //- HA EFFETTO SOLO SE AUTOREFRESH E' ATTIVO (altrimenti non ha senso perchè la richiesta la farebbe get_measure)
                                                                        //- E' UTILE SE SUBITO DOPO VIENE CHIAMATA get_measure
        void wait_new_statistic(); //safe                               //Stessa cosa di wait_new_sample() ma per media e varianza        
        
        
        //METODI SECONDARI (sfruttano i metodi primari)
        double get_measure(){ return convert(get_raw()); };		//Fa la stessa cosa di get_raw(), ma la converte prima di restituirla (ON-THE-GO CONVERSION)
        virtual string stype()=0;	//returns a string explaining the type of sensor
        virtual string sunits()=0;	//returns a string explaining the units of measure used
        void display_measure(){ cout<<stype()<<": "<<get_measure()<<" "<<sunits()<<endl; };
        void display_statistic()
        {
        	lock_guard<mutex> access(rw);
        	cout<<stype()<<":"<<endl<<"Media: "<<statistic.average<<" "<<sunits()<<endl<<"Varianza: "<<statistic.variance<<" "<<sunits()<<endl;
        };
        void plug_to(const Driver<measure_struct,uint16_t>& new_board);//Associa un driver (e la sua request()) al sensore virtuale da chiamare a ogni sample() --> //safe
	virtual string getTimeStamp()	//IN REALTA' INUTILE..
		{
			time_t now;
   			time(&now);
   			char buf[sizeof "2011-10-08T07:07:09Z"];
    			strftime(buf, sizeof buf, "%FT%TZ", gmtime(&now));
 			stringstream ss;
 			string s;
 			ss<<buf;
 			ss>>s;
 			return s;
		}

};

class TempSensor : public Sensor
{
    protected:   
        virtual int mtype(){ return TEMPERATURE; };
        virtual double convert(const uint16_t);                                       //TO IMPLEMENT!!
    public:
        TempSensor(const int refresh_rate, const int avg_interval, const bool enable_autorefresh = true) : Sensor(refresh_rate,avg_interval,enable_autorefresh) {};
        virtual string stype(){ string st="Temperatura"; return st; };
        virtual string sunits(){ string st="°C"; return st; };

};

class HumidSensor : public Sensor
{
    protected:
        virtual int mtype(){ return HUMIDITY; };      
        virtual double convert(const uint16_t);                                       //TO IMPLEMENT!!        
    public:
        HumidSensor(const int refresh_rate, const int avg_interval, const bool enable_autorefresh = true) : Sensor(refresh_rate,avg_interval,enable_autorefresh) {};
        virtual string stype(){ string st="Umidita'"; return st; };
        virtual string sunits(){ string st="%"; return st; };        

  
};

class DustSensor : public Sensor
{
    protected:
        virtual int mtype(){ return DUST; };  
        virtual double convert(const uint16_t);                                       //TO IMPLEMENT!!    
	virtual void set_offset(); 
    public:
        DustSensor(const int refresh_rate, const int avg_interval, const bool enable_autorefresh = true) : Sensor(refresh_rate,avg_interval,enable_autorefresh) {};
        virtual string stype(){ string st="Polveri"; return st; };
        virtual string sunits(){ string st="mg/m^3"; return st; };      

};



/////////////////
#endif

