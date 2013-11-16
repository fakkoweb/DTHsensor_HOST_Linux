
#include "control.h"
#include "hidapi.h"



int control::scan()
{
	struct hid_device_info* devices;	//Lista linkata di descrittori di device (puntatore al primo elemento)
	struct hid_device_info* curr_dev;	//Descrittore di device selezionato (puntatore per scorrere la lista sopra)

	//Scansione della nostra device
	int i, esito_funzione=ERROR;		
	bool trovata=false;
	cout<<"Scansione device in corso..."<<endl;
	while (!trovata)
	{
		//Scansiona tutte periferiche e le alloca in devices
		devices = hid_enumerate(0x0, 0x0);
		curr_dev=devices;
		i=0;
		while(curr_dev) 		//finchè non raggiungo la fine della lista (cioè curr_dev==NULL)
		{
			if(curr_dev->vendor_id == MY_VID && curr_dev->product_id == MY_PID)
			{
				
				/*
				cout<<"Device "<<++i<<" trovata!"<<endl;
				cout<<"  Manufacturer: "<<curr_dev->manufacturer_string<<"\nProduct: "<<curr_dev->product_string<<endl;
				*/
				cout<<"Device "<<++i<<" riconosciuta!"<<endl;
				cout<<"|  VID: "<<hex<<curr_dev->vendor_id<<" PID: "<<curr_dev->product_id<<dec<<endl;
				cout<<"|  Path: "<<curr_dev->path<<"\n|  serial_number: "<<curr_dev->serial_number<<endl;
				cout<<"|  Manufacturer: "<<curr_dev->manufacturer_string<<endl;
				cout<<"|  Product:      "<<curr_dev->product_string<<endl;
				cout<<"|  Release:      "<<curr_dev->release_number<<endl;
				cout<<"|  Interface:    "<<curr_dev->interface_number<<endl;
				trovata=true;
				esito_funzione=NICE;
			}
			else 
			{
				cout<<"Device "<<++i<<"-> VID: "<<curr_dev->vendor_id<<" PID: "<<curr_dev->product_id<<"  --  NO MATCH"<<endl;
			}
			curr_dev=curr_dev->next;
		}
		
		//Dealloca la lista di devices
		hid_free_enumeration(devices);
		
		
		//FLAG DI CONTROLLO
		//*x* Forza il thread a terminare la ricerca se STOP è alto
		if(get_stop())
		{
			trovata=true;		//*x* Forza trovata=true per uscire dal ciclo
			cout<<".....scan aborted by user....."<<endl;
			esito_funzione=ABORTED;
			set_stop(false);	//Resetta il flag
		}
						
		//Se STOP è falso e trovata è falso aspetta 3 secondi prima di effettuare una nuova scansione
		if(!trovata)
		{
			cout<<".....not found....."<<endl;
			p_sleep(5000);
		}
		
		
		
	}
	
	return esito_funzione;	
		
		
}

int control :: read_show(int times, int delay)		//uses recv_measure() and displays/uses its result
{
	int i=0;
	int status=ERROR;
	measure_struct misura;
	
	cout<<"Inizio apertura device..."<<endl;
	hid_device* handle = hid_open(MY_VID, MY_PID, NULL);		//Device in uso (puntatore alla handle del kernel)
	
	if (handle == NULL)
	{
		cout<<"Errore nell'apertura della device!"<<endl;
		status=ERROR;
				hid_close(handle);
	}
	else
	{
		cout<<"Periferica aperta!"<<endl;
		
		for(i=0;i<times && status!=ERROR && status!=ABORTED;i++)
		{
			cout<<"| Lettura da "<<handle<<" in corso..."<<endl;
			status=recv_measure(handle,misura);
			if(status==ERROR) cout<<"| Errore: lettura abortita. Codice: "<<status<<endl;
			else if(status==ABORTED) cout<<"| Lettura abortita dall'utente. Codice: "<<status<<endl;
			else
			{
				cout<<"| Lettura effettuata.\n|   Polvere: "<<misura.dust<<"\n|   Temperatura: "<<misura.temp<<"\n|   Umidità: "<<misura.humid<<endl;	
				p_sleep(delay);
			}
		}

		hid_close(handle);
		cout<<"Device chiusa."<<endl;
	}

	return status;	

}


int control::recv_measure(hid_device* d, measure_struct& m)	//copies device format data into measure_struct data type
{	
	int bytes_read=0,last_bytes_read=0,bytes_to_read=sizeof(measure_struct),i=0,status=ERROR;
	unsigned char buf[bytes_to_read];
	
	
	if(d!=NULL)
	{
		cout<<"  D| Procedura di lettura iniziata."<<endl;
		hid_set_nonblocking(d,1);					//Default - read bloccante (settare 1 per NON bloccante)
	    	while( last_bytes_read != bytes_to_read && bytes_read!=-1 )	//Questo ciclo si ripete finché ho letture errate E SOLO SE NON ho un errore critico (-1) -- !get_stop() && 
		{
	    		cout<<"   D! Tentativo "<<++i<<endl;
	    		do
	    		{
	    			last_bytes_read=bytes_read;						//Memorizza il numero di byte letti dal report precedente
	    			bytes_read = hid_read(d,buf,bytes_to_read);				//Leggi il report successivo:
	    			//cout<<bytes_read<<endl;						//	- se bytes_read>0 allora esiste un report più recente nel buffer -> scaricalo
	    												//	- se bytes_read=0, non ci sono più report -> l'ultimo scaricato è quello buono
	    												//	- se bytes_read=-1 c'é un errore critico con la device -> interrompi tutto
	    												//La read si blocca AL PIU' per 5 secondi, dopodiché restituisce errore critico (-1)
	    		}while(bytes_read>0);								//Cicla finché il buffer non è stato svuotato (0) oppure c'è stato errore critico (-1)
	    		if (bytes_read == -1)								//Se c'è stato errore...
			{
			    cout<<"   D! Lettura fallita."<<endl;
			    cout<<"   D! ERRORE: Periferica non pronta o scollegata prematuramente.\n  D| Disconnessione in corso..."<<endl;
			    hid_close(d);									//chiudi la handle. 	-> ESCE dal ciclo.
	    	            cout<<"  D| Device disconnessa."<<endl;
			    d=NULL;	//Resets handle pointer for safety
			}
	    		else										//..altrimenti analizziamo per bene l'ultimo report scaricato...
	    		{
		    		if (last_bytes_read < bytes_to_read)						//se i byte letti dell'ultima misura nel buffer (la più recente) non sono del numero giusto
		    			cout<<"   D! Lettura fallita."<<endl;					//-> la lettura è fallita, bisogna riprovare. 	-> NON ESCE dal ciclo.
				else										//altrimenti -> stampa a video il risultato e memorizzalo in "m" -> ESCE dal ciclo.
				{
					cout<<"   D! "<<last_bytes_read<<" bytes letti: ";
					cout<<(int)buf[0]<<" "<<(int)buf[1]<<" "<<(int)buf[2]<<" "<<(int)buf[3]<<" "<<(int)buf[4]<<" "<<(int)buf[5]<<" "<<endl;
					memcpy( (void*) &m, (void*) buf, bytes_to_read);
			    		status=NICE;								//In conclusione, la funzione ritorna NICE solo se ha letto esattamente 6byte
				}
			}

        	}

	    	cout<<"  D| Procedura di lettura conclusa."<<endl;
	}
	else cout<<"  D| ERRORE: il driver non è connesso ad alcuna periferica! (device=NULL)"<<endl;
    
	
	return status;				//Qui ERROR è ritornato di default a meno che non vada tutto OK.


}
	
