
#include "control.h"
#include "curl/curl.h"
#include <map>

	


control::control()
{
	stop = false;
	keep_console = true;


	//Creazione del processo "finestra"
	window=vfork();
	if (window==-1)
	{
		fprintf(stderr, "generazione del processo fallita\n");	
	}
	if  (window==0)
	{
	
		char* arg[2];					// La execvp vuole un array di puntatori a char
		const char* fav_terminal = getenv("TERM");	// Ottengo il percorso relativo all'emulatore di terminale preferito dall'utente
		arg[0]= getenv("TERM");				// Il primo deve essere il nome dell'eseguibile stesso
		arg[1]=(char *)0;				// L'ultimo di essi deve essere un puntatore a null char
		
		cout<<"Apertura console "<<fav_terminal<<endl;
		
		
		//CALL OF THE TERMINAL - Apre una finestra "terminale" su cui mandare l'output del cout
		execvp(fav_terminal,arg);  			// eseguo il comando: nuova finestra terminal aperta.
								// Su questa verrà redirezionato l'output standard (cout<<)
		
	}
	else if (window>0)
	{

		        
		p_sleep(2000);        
		        
		
		
		//CALL OF THE CONSOLE - Avvia il thread console
		c= new thread (&control::open_console,this);	// Per eseguire open_console() è richiesto this quando è un
								// metodo della classe stessa


		//REDIREZIONE STANDARD OUTPUT (cout) PER IL THREAD PRINCIPALE verso la nuova finestra!!
		//Queste righe ottengono il percorso del terminale principale (può cambiare da sistema a sistema)
		char main_terminal_path[30];
		sprintf(main_terminal_path, "%s", ttyname(0));
		//cout<<"Terminale attuale: "<<main_terminal<<endl;
		
		//Queste righe estraggono il numero del terminale principale (quello principale)
		char* token;token=strtok(main_terminal_path,"/pts");		
		token=strtok(NULL,"/pts");
		int num_main_terminal=atoi( token );
		//cout<<"Numero terminale attuale: "<<num_main_terminal<<endl;
		
		//Otteniamo dunque il percorso del terminale secondario incrementando il numero principale di uno
		//Assieme al percorso è possibile conoscere il path del file associato alla terminale appena aperto
		sprintf(new_terminal_path, "/dev/pts/%d", num_main_terminal+2);
		//cout<<"Nuovo terminale: "<<new_terminal_path<<endl;
		
		new_terminal_out= new ofstream (new_terminal_path,ios::out);		//Nuovo oggetto stream in uscita (associato alla nuovo terminale)
		//streambuf* main_window_stream_backup = cout.rdbuf();			//Backup dello stream corrente di cout (per ora non necessario)	
		cout.rdbuf(new_terminal_out->rdbuf());					//Redirect cout a new_terminal_out
		cout<<"\n\n LOG \n-----"<<endl;
		//cout.rdbuf(main_window);						//Decommenta questa istruzione (e quella sopra) per ripristinare il backup
		
		
		
		

	}
	
}


void control::open_console()
{
	int choice;
	char command[20];
	const char quit_command[5]="quit";
	
	
	//Mappa dei comandi possibili
	map <string, int> cmdmap; 
	typedef pair <string, int> cmdrow; 
	cmdmap.insert ( cmdrow ( "stop", 1 ) ); 
	cmdmap.insert ( cmdrow ( "usb.refresh", 2 ) );
	
	
	//CREAZIONE CONSOLE OUTPUT!! - output esplicitamente diretto alla finestra principale
	//Il comando "console<<" sarà disponibile per questo thread finchè la console sarà attiva!
	ofstream console(ttyname(0),ofstream::out);		//Nuovo oggetto stream in uscita (associato al terminale principale)
	int i=0;
	
	console<<"\nCONSOLE DI DEBUG\n-------------"<<endl;
	console<<"Comandi:\nquit - esci dal programma (termina tutte le procedure!)\nstop - termina operazione / esci dal loop corrente"<<endl;
	console<<"> ";
	cin.getline(command,20);
	while( strcmp(command,quit_command) != 0 )
	{
		choice=cmdmap[command];	 			//cmdmap() restituisce il valore associato a quella chiave, 0 se non presente
		switch ( choice )
		{
			case 0:
			  console<<"comando errato o non presente"<<endl;
			  break;
			case 1:
			  set_stop(true);
			  console<<"Comando di abort mandato"<<endl;
			  while(get_stop() == true && i<5)	//Se il flag di stop non è ancora stato resettato dalla funzione 
			  {					//chiamata (DEVE FARLO) significa che il comando NON è stato ricevuto
			  	i++;
			  	console<<"."<<endl;
			  	p_sleep(1000);			  	
			  }
			  if(get_stop() == true) console<<"...ma la funzione non risponde.\nSicuro ci sia qualche procedura in esecuzione?"<<endl; //Al quinto tentativo fa l'ultima verifica
			  else console<<"comando ricevuto!"<<endl;
			  i=0;
			  break;
			case 2:
			  console<<"riavvia la scansione usb (se non già in corso)"<<endl;
			  break;
			default:
			  console<<"ERRORE"<<endl;
			  break;
		}
		console<<"> ";
		cin.getline(command,20);
	}
	
	set_stop(true);		//Se la console si chiude, tutte le routine terminano! (per default)
	console.close();	//Chiude esplicitamente l'oggetto stream in uscita
	close_console();	//*x* Metto il flag di keep_console su FALSE (per motivi di semantica)	
	return;
}



control::~control()
{

	
	
	if( console_is_open() ) cout<<"Confermare la chiusura programma da console con 'quit'."<<endl;
	c->join();

	cout<<"Chiusura della finestra."<<endl;
	kill(window,SIGKILL);				//La finestra è un processo che non esegue nulla. 
							//Anche se poco ortodossa, la kill è facile e non ci sono danni.
	
	int s;
	wait(&s);
	//Manca il controllo sulla chiusura della finestra...
	
	
	//Deallocate CURL library
	curl_global_cleanup();
	

}


