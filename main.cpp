#include <cstdlib>
#include <cstdio>
#include <iostream>

//Load user configuration and define variables
#include "control.h"

//Include standard functions
#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
    #include <string.h>
#endif
#include <map>

//Include components
#include "sensors.h"    
#include "drivers.h"
#include "control.h"    //CLASS for debug
                        //-- Just define it at beginning of main and all will under control!
#include "functions.h"  //FUNCTIONS (including for Curl and Json)
                        //-- ALERT: libcurl needs to be initialized manually with curl_global_init()!!

using namespace std;




typedef struct _PARAM_STRUCT
{
    int MY_VID ;
    int MY_PID ;
    int EXT_TEMP_lfid ;
    int EXT_HUMID_lfid ;
    int EXT_DUST_lfid ;
    int INT_TEMP_lfid ;
    int INT_HUMID_lfid ;
    int TEMP_REFRESH_RATE ;
    int HUMID_REFRESH_RATE ;
    int DUST_REFRESH_RATE ;
    int REPORT_INTERVAL ;
} param_struct;




int main(int argc, char* argv[])
{
    
    /////////////////////////////////////////////////////////
    //INIZIALIZZAZIONE LIBRERIE DI UTILITA'
    /////////////////////////////////////////////////////////
    //ATTIVARE DEBUG??
    //Control program;
    
    //Inizializzazione CURL library -- necessario
	curl_global_init(CURL_GLOBAL_ALL);
    






    /////////////////////////////////////////////////////////
    //LETTURA DEI PARAMETRI UTENTE E ANNUNCIO AL SERVER
    /////////////////////////////////////////////////////////

    //Registrazione device
    int device_id = register_device(MY_VID,MY_PID);
    
    //Registrazione sensori (da parameters.json)
    param_struct user_config;
    if ( param_load(user_config,"parameters.json") != NICE ) return 1;
    



    
    
    
    ///////////////////////////////////////////////////////
    //DESCRIZIONE DELLA CONFIGURAZIONE FISICA DEL SISTEMA
    ///////////////////////////////////////////////////////
    
    //Inizializzazione driver virtuali
    Raspberry::init();
    Usb::init();
    
    //Creazione dei sensori virtuali
    param_struct* p = &user_config;
    TempSensor exttemp( p->TEMP_REFRESH_RATE, p->REPORT_INTERVAL );     //Impostiamo l'intervallo in cui il sensore calcola la media come
    TempSensor inttemp( p->TEMP_REFRESH_RATE, p->REPORT_INTERVAL );     //l'intervallo in cui mandare i report al server. In questo modo,
    HumidSensor inthumid( p->HUMID_REFRESH_RATE, p->REPORT_INTERVAL );  //circa ogni REPORT_INTERVAL, avremo medie e varianze pronte.
    HumidSensor exthumid( p->HUMID_REFRESH_RATE, p->REPORT_INTERVAL );
    DustSensor extdust( p->DUST_REFRESH_RATE, p->REPORT_INTERVAL );
    
    //Allacciamento dei sensori
    exttemp.plug_to(Usb::isr());
    exthumid.plug_to(Usb::isr());
    extdust.plug_to(Usb::isr());
    inttemp.plug_to(Raspberry::isr());
    inthumid.plug_to(Raspberry::isr());
    
    //Associazione dei local_feed_id scelti ai giusti sensori
   	map <int, Sensor*> SensorArray; 
	typedef pair <int, Sensor*> new_row;
	SensorArray.insert ( new_row ( p->EXT_TEMP_lfid, &exttemp ) );
	SensorArray.insert ( new_row ( p->EXT_HUMID_lfid, &exthumid ) );
	SensorArray.insert ( new_row ( p->EXT_DUST_lfid, &extdust ) );
	SensorArray.insert ( new_row ( p->INT_TEMP_lfid, &inttemp ) );
	SensorArray.insert ( new_row ( p->INT_HUMID_lfid, &inthumid ) );
    //In questo modo non è importante come sono posizionati i sensori nell'array
    //Ogni sensore è indicizzato tramite il proprio local_feed_id
    //ATTENZIONE: occorre PRIMA fare la get al server per ottenere/recuperare i local_feed!!
    



    
    /////////////////////////////////////////////////////////////
    //AVVIO ROUTINE E LOOP DI PROGRAMMA
    /////////////////////////////////////////////////////////////
    int state;
    bool exit=false;
    while ( state == NICE && exit == false )
    {
        state=report_routine(device_id,SensorArray);
        //TO IMPLEMENT: exit control on variable!
    }
    return state;
    
    for(i=0;i<5;i++)
    {
        a[i].wait_new_statistic();
    }
    for(i=0;i<5;i++)
    {
        a[i].get_raw_statistic();
    }
    
    
    
    
    /*
	int status;
	control program;		//////////////////////////CLASSE CONTROL////////////////////////////
				// La classe Control contiene:
				//	- TUTTE le funzioni() USB del programma (per il momento è l'unica)
				//	- una console utente per controllare e interagire con le funzioni attraverso flag
				//	- flag per il controllo di flusso delle funzioni
				//	- metodi() per interrogare i flag (le funzioni DEVONO chiamarli ove necessario)
				// Nel costruttore:
				//	- vengono inizializzati i flag
				//	- viene creato un thread per la console (vedi funzione open_console())
				//	- viene aperta una finestra separata per l'output del programma
				//	- TUTTO l'output principale del programma (cout<<) è redirezionato sulla nuova finestra
				//	- SOLO l'output della console (console<<) resta sulla finestra principale
				// Nel distruttore (invocato automaticamente alla fine del programma)
				//	- viene chiesta conferma all'utente di terminare il programma (digitando "quit")
				//	- viene terminato il thread per la console
				//	- (NON IMPLEMENTATO) viene ripristinato l'output principale del programma sulla finestra principale
				//	- viene chiusa la finestra
				// NOTA: la classe funziona solo su sistemi UNIX (per via della vfork() e della kill())
				


    //LOOP DI PROVA
	do{

		cout<<"Procedura scansione usb iniziata."<<endl;			
		status=usb::scan();	// Scansiona le periferiche usb fino a che:
					//	- individua una periferica con VID e PID noti (manca il valore di ritorno)
					//	- la procedura è interrotta dall'utente
					// Ritorna l'esito funzione nel codice espresso in control.h
	
		if(status==NICE)
		{	
			cout<<"Procedura di lettura iniziata."<<endl;
			do
			{
				status=usb::read_show(10,1000);
				cout<<"Status ricevuto dalla read= "<<status<<endl;
			}while(status!=ABORTED && status!=ERROR);	// Ferma il loop di lettura da console
		}
	
		//Altre procedure;
	
		//usb.manual(); lascia all'utente la liberta di scegliere cosa attivare (magari con un flag)
	
	
	
	} while(status!=ABORTED);
	
	
	
	//IDLE LOOP
	measure_struct outer, inner;
	
	cout<<"Procedura scansione usb iniziata."<<endl;			
	status=usb::scan();	// Scansiona le periferiche usb fino a che:
    					//	- individua una periferica con VID e PID noti (manca il valore di ritorno)
    					//	- la procedura è interrotta dall'utente
    					// Ritorna l'esito funzione nel codice espresso in control.h
    	
	if(status==NICE)
	{
	    hid_device* handle = hid_open(MY_VID, MY_PID, NULL);
	    do{

		    cout<<"Procedura di lettura usb"<<endl;
		    if ( usb::recv_measure(handle, outer) == NICE && rasp::recv_measure(inner) == NICE )
		    {
				convert(outer);
				convert(inner);
				send_report();
				if( program.get_stop() ) status=ABORTED;
			}
			else status=ERROR;
			
		} while(status!=ABORTED);
	
	}

	
	
	cout<<"La ricerca periferica si riavvierà fra 5 secondi..."<<endl;
	p_sleep(5000);
	

    */
    
	return 0;
	//viene chiamato il distruttore di usb

}

