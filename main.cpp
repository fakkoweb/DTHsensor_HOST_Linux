

//Load user configuration and define variables
#include "config.h"

//Include standard functions
#include <cstdlib>
#include <cstdio>
//#include <fstream>
#include <iostream>
#include <ctime>
#include <map>
#include <list>
#include <future>
//#include <chrono>

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
#include "json.h"	//For Dynamic Parameter loading from a .json file
//#include "http_manager.h"


using namespace std;









/////////////////////////////////////////////////////////
//LETTURA DEI PARAMETRI UTENTE
/////////////////////////////////////////////////////////

const Json::Value params = load_params("parameters.json");   	// Albero di valori json estratti da "parameters.json" a runtime
								// GLOBALE perchè deve essere accessibile a tutto il progetto!
								// COSTANTE perchè nessuna parte di progetto deve comprometterla!




int main(int argc, char* argv[])
{
    
    /*
    if ( argc>2 || strlen(argv[1])>2 )
    {
	printf("usage: %s [-v]\n", argv[0]);
	exit(0);	 
    }
    */
    
    if (params.empty())
    {
    	cout<<"Parameters corrupted or unavailable. Exiting..."<<endl;
    	return 1;
    }
    
    
    
    /////////////////////////////////////////////////////////
    //INIZIALIZZAZIONE LIBRERIE DI UTILITA' E OUTPUT
    /////////////////////////////////////////////////////////
    //ATTIVARE DEBUG??
    //Control program;
    
    //Inizializzazione CURL library -- necessario
    curl_global_init(CURL_GLOBAL_ALL);
    cout<<"Librerie inizializzate"<<endl;
    
    
    
    // LOG FILE MANUAL REDIRECTION
    /*
    ofstream log_out("log"+getTimeStamp()+".txt",ios::out | ios::app);			//Nuovo oggetto stream in uscita (associato al file di log)
    //if(argc<2 || strcmp("-v",argv[1])!=0)
    //{
    	    cout<<"Log di cerr redirezionato su log.txt"<<endl;
	    //streambuf* main_window_stream_backup = cout.rdbuf();			//Backup dello stream corrente di cerr (per ora non necessario)	
	    cerr.rdbuf(log_out.rdbuf());						//Redirect cerr a log_out
	    cerr<<"\n\n ----- LOG "<<getTimeStamp()<<" -----"<<endl;
	    //cout.rdbuf(main_window);
    //}
    */
    
    /* PUT THIS HERE FOR LOG DELAY FILE CREATION
    std::chrono::duration< int, std::milli > new_log_delay;
    std::chrono::steady_clock::time_point new_log_created = std::chrono::steady_clock::now();
    */
    
    		/* THEN PUT THIS IN LOOP
		if (std::chrono::steady_clock::now() <= (new_log_created+new_log_delay) )
		{
			//delay_elapsed = false;
		}
		else
		{
			//delay_elapsed = true;
			log_out.close();
			//remove(from_filename.c_str());
			log_out.open("log"+getTimeStamp()+".txt",ios::out | ios::app);
			new_log_created = std::chrono::steady_clock::now();
		}
		*/

   
    
    /////////////////////////////////////////////////////////
    //CREAZIONE STRUTTURE DI INDICIZZAZIONE E THREADING
    /////////////////////////////////////////////////////////    
    //Creazione delle strutture di indicizzazione locale
    map <int, Sensor*> AllSensors;		//Indice di tutti i sensori associati al proprio lfid
    map <int, Sensor*> ExtSensors;		//Indice dei soli sensori esterni
    map <int, Sensor*> IntSensors; 		//Indice dei soli sensori interni
    map<int, Sensor*>::iterator row;		//Iteratore generico per accedere alle map   
    typedef pair <int, Sensor*> new_row;	//Tipo riga per inserimento nelle map

    //Creazione delle strutture di threading per l'ottimizzazione
    list<thread*> Waiters;			//Lista di thread per attendere (in parallelo) le statistiche prodotte dal sensore per "apparecchiarle"
    future<bool> server_sync_status;		//Risultato ritornato dalla async() per le operazioni di rete.
    						//LA async() VIENE AVVIATA PRIMA DELL'ATTESA DELLE STATISTICHE, ALLA FINE DELLA QUALE SARA' VALUTATO IL RISULTATO!
    						//Ritorna false se c'è stato un fallimento di rete qualsiasi.
    bool ready_to_post=false;			//Mantiene in memoria L'ULTIMO VALORE ritornato da server_sync: indica se si può cominciare a postare le statistiche
//  thread* Registering_thread=NULL;		//Thread "joinable" per la registrazione della device/sensori al server
    						//In questo modo, mentre avviene la comunicazione col server, posso cominciare ad attendere le statistiche.
    						//DEVE essere avviato prima di Reporting_thread (ho bisogno di sapere se posso postare o meno le misure)

//  thread* Reporter=NULL;			//Thread separato (che sarà "detached") per postare le statistiche attese dai sensori
    						//IL thread VIENE AVVIATO SUBITO DOPO L'ATTESA DELLE STATISTICHE, MA NON VERRA' ATTESA LA SUA TERMINAZIONE!
    //PSEUDO-SCHEMA DI ESECUZIONE:
    /*	thread principale	
    	server_sync=async()	->	thread di rete			+1 thread
    	Waiters(sensori)	->	thread(s) di attesa		+n thread (uno per sensore)
	Waiters.join()		<-					-n thread (uno per sensore)
    	server_sync.get()	<-					-1 thread
    */
  				    
    
    
    
    ///////////////////////////////////////////////////////
    //DESCRIZIONE DELLA CONFIGURAZIONE FISICA DEL SISTEMA
    ///////////////////////////////////////////////////////
    
    //Inizializzazione driver virtuali
    Raspberry int_device;
    Usb ext_device(1240,19);
    cout<<"Driver pronti"<<endl;
    
    //Creazione dei sensori virtuali
    //Prototipo: Sensor s( sample_rate , interval_for_average , autosample ); -> (millisecondi, minuti, bool)
    TempSensor exttemp( params["sensors"]["temp"].get("REFRESH_RATE",0).asInt(), params["report"].get("INTERVAL",0).asInt(), true );	 //Impostiamo il periodo su cui il sensore calcola la media (in minuti)
    TempSensor inttemp( params["sensors"]["temp"].get("REFRESH_RATE",0).asInt(), params["report"].get("INTERVAL",0).asInt(), true );     //uguale all'intervallo in cui dobbiamo mandare i report al server.
    HumidSensor inthumid( params["sensors"]["humid"].get("REFRESH_RATE",0).asInt(), params["report"].get("INTERVAL",0).asInt(), true );  //In questo modo, ogni REPORT_INTERVAL, avremo medie e varianze pronte.
    HumidSensor exthumid( params["sensors"]["humid"].get("REFRESH_RATE",0).asInt(), params["report"].get("INTERVAL",0).asInt(), true );
    DustSensor extdust( params["sensors"]["dust"].get("REFRESH_RATE",0).asInt(), params["report"].get("INTERVAL",0).asInt(), true );
    cout<<"Sensori virtuali pronti"<<endl;
		
    //Indicizzazione locale dei sensori (local_feed_id <-> sensor)
    AllSensors.insert ( new_row ( params["sensors"]["temp"]["ext"].get("lfid",0).asInt(), &exttemp ) );
    ExtSensors.insert ( new_row ( params["sensors"]["temp"]["ext"].get("lfid",0).asInt(), &exttemp ) );
    
    AllSensors.insert ( new_row ( params["sensors"]["humid"]["ext"].get("lfid",0).asInt(), &exthumid ) );
    ExtSensors.insert ( new_row ( params["sensors"]["humid"]["ext"].get("lfid",0).asInt(), &exthumid ) );

    AllSensors.insert ( new_row ( params["sensors"]["dust"]["ext"].get("lfid",0).asInt(), &extdust ) );
    ExtSensors.insert ( new_row ( params["sensors"]["dust"]["ext"].get("lfid",0).asInt(), &extdust ) );

    AllSensors.insert ( new_row ( params["sensors"]["temp"]["int"].get("lfid",0).asInt(), &inttemp ) );
    IntSensors.insert ( new_row ( params["sensors"]["temp"]["int"].get("lfid",0).asInt(), &inttemp ) );
	
    AllSensors.insert ( new_row ( params["sensors"]["humid"]["int"].get("lfid",0).asInt(), &inthumid ) );
    IntSensors.insert ( new_row ( params["sensors"]["humid"]["int"].get("lfid",0).asInt(), &inthumid ) );
    //N.B. non è importante come sono posizionati i sensori nella map:
    //ogni sensore è localmente identificato e accessibile tramite il proprio local_feed_id!

    //Display indice dei sensori allocati
    cout<<"-- Sensori disponibili --"<<endl;
    cout<<"| ID\t| SENSOR\t| TYPE"<<endl;
    for (row=AllSensors.begin(); row!=AllSensors.end(); row++)
    {
	cout<<"| "<< row->first <<"\t| "<< (size_t)row->second <<"\t| "<< row->second->stype() <<endl;	
    }

    cout<<"Driver e sensori virtuali istanziati e pronti. Allacciamento tra\n3.."<<endl;
    p_sleep(1000);
    cout<<"2.."<<endl;
    p_sleep(1000);
    cout<<"1.."<<endl;
    p_sleep(1000);
    
    //Allacciamento dei sensori ai driver virtuali (alias board) con un time_point comune
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    exttemp.plug_to(ext_device,now);
    exthumid.plug_to(ext_device,now);
    extdust.plug_to(ext_device,now);
    inttemp.plug_to(int_device,now);
    inthumid.plug_to(int_device,now);
    //Il parametro "now" serve a dare un riferimento temporale unico tra i sensori, eliminando così il jitter relativo tra di essi
    cout<<"Sensori allacciati ai driver. Letture iniziate."<<endl;
     
    

    cout<<"MAIN: Avvio loop principale...\n----------------------"<<endl;

    
    //TEST LOOP - CANCELLARE QUESTO LOOP QUANDO IL RESTO DEL PROGRAMMA E' IMPLEMENTATO
    if (!AllSensors.empty())
    {
	while(1)
	{
				



		
		///////////////////////////////////////////////////////
		//SINCRONIZZAZIONE CON IL SERVER (Avvio Thread)
		///////////////////////////////////////////////////////
		//Primo ciclo: ready_to_post è FALSE, quindi avverrà SEMPRE PRIMA la fase di registrazione al server
		//Prossimi cicli:
		//	- fase di registrazione andata a buon fine: ready_to_post è TRUE -> avvio post dei report
		//	- fase di registrazione fallita: ready_to_post è FALSE -> riavvio fase di registrazione
		//	- fase di post report fallita: ready_to_post è resettato a FALSE -> riavvio fase di registrazione
		//L'algoritmo è ottimizzato per effettuare le operazioni di rete DURANTE LE ATTESE delle statistiche successive.
		//Il fallimento della rete non pregiudica il corretto salvataggio delle statistiche in locale!!
		
		if(!ready_to_post)
		{
			cout<<"MAIN: Avvio thread di routine annuncio al server..."<<endl;
			server_sync_status = async(std::launch::async, registering, params["device"].get("MY_MAC",0).asString(), params["sensors"]);
		}
		else
		{
			cout<<"MAIN: Avvio thread di routine sincronizzazione con server..."<<endl;
			server_sync_status = async(std::launch::async, reporting, "awaiting_reports.txt", params["device"].get("MY_MAC",0).asString(), AllSensors);
		}
		
		//Registering_thread = new thread (registering, params["device"].get("MY_MAC",0).asString(), params["sensors"]);	// post_report() bufferizza SEMPRE le misure ma le manda al server solo se ready_to_post è TRUE
		





		///////////////////////////////////////////////////////
		//ATTESA DELLE MISURE DAI SENSORI
		///////////////////////////////////////////////////////		
		row=AllSensors.begin();

		//Creazione della coda camerieri, ognuno in attesa su un sensore
		cout<<"MAIN: Avvio creazione dei Waiters..."<<endl;
		for (row=AllSensors.begin(); row!=AllSensors.end(); row++)
		{
			Waiters.push_back
			(
				new thread
					(
						[](Sensor* s) -> void
						{
							cout<<"WAITER: Waiter creato e in attesa di nuova statistica..."<<endl;
							s->wait_new_statistic();
							cout<<"WAITER: Statistica pronta: termino."<<endl;
							return;
						}
					, row->second)
			);
		}

		//Attesa dei camerieri (dal primo all'ultimo) che ritornano quando statistica pronta
		cout<<"MAIN: Attento la terminazione dei Waiters..."<<endl;
		while ( !Waiters.empty() )
		{
			Waiters.front()->join();
			Waiters.pop_front();
		}
		cout<<"MAIN: Terminazione dei Waiters completata. Le statistiche sono pronte per essere prelevate dai sensori."<<endl;
		
		
		
		//**** QUI ARRIVO SOLO QUANDO IL MAIN HA LA STATISTICA PRONTA PER TUTTI I SENSORI!!! ****
		
		
		
		
		
		
		///////////////////////////////////////////////////////
		//OUTPUT DI CONTROLLO STATISTICHE (per il debug)
		///////////////////////////////////////////////////////	
		//OUTPUT DI DEBUG (verifica delle statistiche rilevate a video)
		//Prova di uso e consumo delle statistiche
		if(!IntSensors.empty())
		{
			cout<<"Misure interne:"<<endl;
			for (row=IntSensors.begin(); row!=IntSensors.end(); row++)
			{	
				cout<<"| Sensor Local Id: "<< row->first <<endl;
				cout<<"| Average: "<< row->second->get_statistic().average << " Variance: " << row->second->get_statistic().variance << endl;
				cout<<"| With "<< row->second->get_statistic().valid_samples <<"/"<< row->second->get_statistic().total_samples<<" valid/total samples taken."<<endl;
				cout<<"| Expected "<< row->second->get_statistic().expected_samples <<" valid samples, therefore overall statistic is "<< row->second->get_statistic().percentage_validity <<"% valid"<<endl;
				cout<<"| Statistic has to be considered as ";
				if (row->second->get_statistic().is_valid){
					cout << "VALID.";
				}
				else{
					cout << "INVALID.";
				}

				cout << endl;
			}
		}
		if(!ExtSensors.empty())
		{
			cout<<"Misure esterne:"<<endl;
			for (row=ExtSensors.begin(); row!=ExtSensors.end(); row++)
			{
				
				cout<<"| Sensor Local Id: "<< row->first <<endl;
				cout<<"| Average: "<< row->second->get_statistic().average << " Variance: " << row->second->get_statistic().variance << endl;
				cout<<"| With "<< row->second->get_statistic().valid_samples <<"/"<< row->second->get_statistic().total_samples<<" valid/total samples taken."<<endl;
				cout<<"| Expected "<< row->second->get_statistic().expected_samples <<" valid samples, therefore overall statistic is "<< row->second->get_statistic().percentage_validity <<"% valid"<<endl;
				cout<<"| Statistic has to be considered as ";
				if (row->second->get_statistic().is_valid){
					cout << "VALID.";
				}
				else{
					cout << "INVALID.";
				}

				cout << endl;
			}		
		}
		
		

		
		
		
		
		/////////////////////////////////////////////////////////////
		//SALVATAGGIO STATISTICHE IN LOCALE
		/////////////////////////////////////////////////////////////
		
		cout<<"MAIN: Avvio prelievo e salvataggio locale delle statistiche..."<<endl;
		if( save_report("awaiting_reports.txt", AllSensors) != NICE )
			cout<<"MAIN: Salvataggio fallito. Statistiche perdute."<<endl;
		else
			cout<<"MAIN: Salvataggio completato."<<endl;
		
		
				
		
		
		
		/////////////////////////////////////////////////////////////
		//VALUTAZIONE SINCRONIZZAZIONE CON SERVER (Chiusura Thread)
		/////////////////////////////////////////////////////////////
		//ATTENZIONE: quando una connessione improvvisamente fallisce, il programma ripeterà l'intera procedura
		//di verifica registrazione al server prima di riprendere a postare i report (ready_to_post=false).
		//Questo per due motivi:
		//	1) il programma non sa per quanto tempo è rimasto scollegato, né se c'è stato un guasto al server
		//	   quindi in ogni caso andrebbe almeno riverificata la validità del server stesso;
		//	2) rifare la verifica evita inutili chiamate alla post_report, la quale effettua SEMPRE un accesso
		//	   e caricamento in memoria di TUTTE le righe salvate e solo POI tenta di mandarle.
		
		if( is_ready(server_sync_status) )
		{
			cout<<"MAIN: Il thread di rete ha concluso l'esecuzione.. ";
			ready_to_post=server_sync_status.get();
			if(ready_to_post) cout<<"e non ha riportato problemi."<<endl;
			else cout<<"e ha riscontrato problemi."<<endl;
		}
			
		
		/* OLD METHOD
		if( ready_to_post )
		{
			if(!ready_to_post) registration_result = async(std::launch::async, registering, params["device"].get("MY_MAC",0).asString(), params["sensors"]);
			if(ready_to_post) cout<<"Le misure saranno spedite sul server appena disponibili"<<endl;
			else cout<<"Le misure saranno bufferizzate.\nLa comunicazione col server sarà riverificata tra circa "<<params["report"].get("INTERVAL",0).asInt()<<" minuti."<<endl;
		}
		//Registering_thread->join();
		//if(Reporting_thread!=NULL) delete Reporting_thread;
		
		Reporter = new thread (reporting, "awaiting_reports.txt", params["device"].get("MY_MAC",0).asString(), AllSensors, ready_to_post);		// post_report() bufferizza SEMPRE le misure ma le manda al server solo se ready_to_post è TRUE
		cout<<"Nuovo thread di report avviato!"<<endl;
		Reporter->detach();
		*/
		

	}
    }
    else cout<<"MAIN: Nessun sensore allocato! Che programma inutile..."<<endl;
	

/*



    ///////////////////////////////////////////////////////
    //ANNUNCIO DEL SISTEMA AL SERVER
    ///////////////////////////////////////////////////////
    int dev_reg_status=ERROR;
    int sens_reg_status=ERROR;
    bool ready_to_post=false;
    

    //Registrazione device - ritorna NICE o ABORTED se ha successo, altrimenti riproverà al prossimo giro
    if(dev_reg_status==ERROR) dev_reg_status = register_device(params["device"].get("MY_MAC",0).asString());
    
    if(dev_reg_status!=ERROR && sens_reg_status==ERROR)
    {
	    //Registrazione sensori - NON ritorna gli unique_feed_id registrati sul server
	    //(1) ATTENZIONE se si cambiano i valori lfid dei sensori (ma non la loro posizione) da parameters.json il sistema sarà diverso!!
	    //(2) ATTENZIONE scambiare gli lfid tra loro mischierebbe le misure al server!
	    sens_reg_status = register_sensors(params["device"].get("MY_MAC",0).asString() , params["sensors"]);
    }
    ready_to_post = (dev_reg_status!=ERROR && sens_reg_status!=ERROR);





    
    /////////////////////////////////////////////////////////////
    //AVVIO ROUTINE E LOOP DI PROGRAMMA
    /////////////////////////////////////////////////////////////
    int state;
    bool exit=false;
    while ( state == NICE && exit == false )
    {
        state=post_report(AllSensors);        //DA IMPLEMENTARE
        //TO IMPLEMENT: exit control on variable!
    }
    return state;
    


*/







/////////////////////

//   OLD VERSION

/////////////////////




    

/*
	int status;
	control program;	//////////////////////////CLASSE CONTROL////////////////////////////
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

