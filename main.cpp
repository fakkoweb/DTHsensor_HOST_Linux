

//Load user configuration and define variables
#include "control.h"

//Include standard functions
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <map>
#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
    #include <string.h>
#endif

//Include components
#include "sensors.h"    
#include "drivers.h"
#include "control.h"    //CLASS for debug
                        //-- Just define it at beginning of main and all will under control!
#include "functions.h"  //FUNCTIONS (including for Curl and Json)
                        //-- ALERT: libcurl needs to be initialized manually with curl_global_init()!!
#include "curl/curl.h"  //For Curl initialization
#include <json.h>	//For Dynamic Parameter loading from a .json file

using namespace std;









/////////////////////////////////////////////////////////
//LETTURA DEI PARAMETRI UTENTE
/////////////////////////////////////////////////////////

const Json::Value params = load_params("parameters.json");   	// Albero di valori json estratti da "parameters.json" a runtime
								// GLOBALE perchè deve essere accessibile a tutto il progetto!
								// COSTANTE perchè nessuna parte di progetto deve comprometterla!




int main(int argc, char* argv[])
{
    
    
    if (params.empty())
    {
    	cout<<"Parameters corrupted or unavailable. Exiting..."<<endl;
    	return 1;
    }
    
    
    
    /////////////////////////////////////////////////////////
    //INIZIALIZZAZIONE LIBRERIE DI UTILITA'
    /////////////////////////////////////////////////////////
    //ATTIVARE DEBUG??
    //Control program;
    
    //Inizializzazione CURL library -- necessario
    curl_global_init(CURL_GLOBAL_ALL);
    cout<<"Librerie inizializzate"<<endl;
    
    
    
    
    
    p_sleep(3000);

   
    
    
    
    
    ///////////////////////////////////////////////////////
    //DESCRIZIONE DELLA CONFIGURAZIONE FISICA DEL SISTEMA
    ///////////////////////////////////////////////////////
    
    //Inizializzazione driver virtuali
    //Raspberry int_device;
    Usb ext_device(1240,19);
    cout<<"Driver pronti"<<endl;
    
    
    //Creazione dei sensori virtuali
    //Prototipo: Sensor s( sample_rate , average_interval ); -> (secondi, minuti)
    TempSensor exttemp( params["sensors"]["temp"].get("REFRESH_RATE",0).asInt(), params["report"].get("INTERVAL",0).asInt(), true );     //Impostiamo il periodo su cui il sensore calcola la media (in minuti)
    //TempSensor inttemp( params["sensors"]["temp"].get("REFRESH_RATE",0).asInt(), params["report"].get("INTERVAL",0).asInt() );     //uguale all'intervallo in cui dobbiamo mandare i report al server.
    //HumidSensor inthumid( params["sensors"]["humid"].get("REFRESH_RATE",0).asInt(), params["report"].get("INTERVAL",0).asInt() );  //In questo modo, ogni REPORT_INTERVAL, avremo medie e varianze pronte.
    HumidSensor exthumid( params["sensors"]["humid"].get("REFRESH_RATE",0).asInt(), params["report"].get("INTERVAL",0).asInt(), true );
    DustSensor extdust( params["sensors"]["dust"].get("REFRESH_RATE",0).asInt(), params["report"].get("INTERVAL",0).asInt(), true );
    cout<<"Sensori virtuali pronti"<<endl;
    
    //Associazione dei local_feed_id ai giusti sensori
   	map <int, Sensor*> SensorArray; 
	typedef pair <int, Sensor*> new_row;
	SensorArray.insert ( new_row ( params["sensors"]["temp"]["ext"].get("lfid",0).asInt(), &exttemp ) );
	SensorArray.insert ( new_row ( params["sensors"]["humid"]["ext"].get("lfid",0).asInt(), &exthumid ) );
	SensorArray.insert ( new_row ( params["sensors"]["dust"].get("lfid",0).asInt(), &extdust ) );
	//SensorArray.insert ( new_row ( params["sensors"]["temp"]["int"].get("lfid",0).asInt(), &inttemp ) );
	//SensorArray.insert ( new_row ( params["sensors"]["humid"]["int"].get("lfid",0).asInt(), &inthumid ) );
    //In questo modo non è importante come sono posizionati i sensori nell'array
    //Ogni sensore è indicizzato tramite il proprio local_feed_id
    //ATTENZIONE: occorre PRIMA fare la get al server per ottenere/recuperare i local_feed!!

    //Display della mappa di sensori appena creata
    std::map<int, Sensor*>::iterator row;
    cout<<"-- Sensori disponibili --"<<endl;
    cout<<"| ID\t| SENSOR\t| TYPE"<<endl;
    for (row=SensorArray.begin(); row!=SensorArray.end(); row++)
    {
	cout<<"| "<< row->first <<"\t| "<< (size_t)row->second <<"\t| "<< row->second->stype() <<endl;	
    }

    p_sleep(3000);
        
    //Allacciamento dei sensori ai driver (alias board)
    cout<<"Sensori allacciati e pronti alla lettura"<<endl;
    exttemp.plug_to(ext_device);
    exthumid.plug_to(ext_device);
    extdust.plug_to(ext_device);
    //inttemp.plug_to(int_device);
    //inthumid.plug_to(int_device);
        
    

    cout<<"Avvio loop principale..."<<endl;
    
    
    //TEST LOOP - CANCELLARE QUESTO LOOP QUANDO IL RESTO DEL PROGRAMMA E' IMPLEMENTATO
	while(1)
	{
		for (row=SensorArray.begin(); row!=SensorArray.end(); row++)
		{
			row->second->wait_new_sample();
		
		}
		for (row=SensorArray.begin(); row!=SensorArray.end(); row++)
		{
			row->second->display_measure();
		
		}

	}
	

/*



    ///////////////////////////////////////////////////////
    //ANNUNCIO DEL SISTEMA AL SERVER
    ///////////////////////////////////////////////////////
    
    //Registrazione device - ritorna la device_id registrata sul server
    int device_id = register_device(params["device"].get("MY_VID",0).asInt() , params["device"].get("MY_PID",0).asInt());   //DA IMPLEMENTARE!!
    
    //Registrazione sensori - NON ritorna gli unique_feed_id registrati sul server
    //(1) ATTENZIONE se si cambiano gli lfid dei sensori da parameters.json il sistema sarà diverso!!
    //(2) ATTENZIONE scambiare gli lfid tra loro mischierà le misure al server!
    //UNA VOLTA REGISTRATA LA DEVICE E I SENSORI occorre cancellarla e registrarla nuovamente.
    register_sensors(device_id,SensorArray,params["sensor"]);                //DA IMPLEMENTARE!!





    
    /////////////////////////////////////////////////////////////
    //AVVIO ROUTINE E LOOP DI PROGRAMMA
    /////////////////////////////////////////////////////////////
    int state;
    bool exit=false;
    while ( state == NICE && exit == false )
    {
        state=post_report(SensorArray);        //DA IMPLEMENTARE
        //TO IMPLEMENT: exit control on variable!
    }
    return state;
    


*/







/////////////////////

//   OLD VERSION

/////////////////////




    
    
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

