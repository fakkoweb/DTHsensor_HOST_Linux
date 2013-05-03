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

	control usb;		//////////////////////////CLASSE CONTROL////////////////////////////
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
				

	cout<<"Procedura scansione usb iniziata."<<endl;			
	usb.scan();		// Scansiona le periferiche usb fino a che:
				//	- individua una periferica con VID e PID noti (manca il valore di ritorno)
				//	- la procedura è interrotta dall'utente
				// Ritorna l'esito funzione nel codice espresso in control.h
	
	
	cout<<"Procedura di lettura iniziata."<<endl;
	while(!usb.get_stop())	// Ferma il loop di lettura da console
	{
		usb.read_show();
		p_sleep(2000);
	}
	usb.set_stop(false);	// Resetta il flag di stop per segnalare alla console il messaggio ricevuto
	
	
	//Altre procedure;
	
	//usb.manual(); lascia all'utente la liberta di scegliere cosa attivare (magari con un flag)
	
			
return 0;

}

