#include <cstdlib>
#include <cstdio>
#include <iostream>
#include "hidapi.h"
#include "control.h"


#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
    #include <string.h>
#endif


using namespace std;




int main(int argc, char* argv[])
{
    
    TempSensor exttemp;
    exttemp.plug_to(Raspberry::isr());
    exttemp.convert(1);
    
    
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

