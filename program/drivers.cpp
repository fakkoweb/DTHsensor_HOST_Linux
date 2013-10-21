
#include "drivers.h"
#include "hidapi.h"


///////////////////////////
//GENERIC DRIVER PROCEDURES
bool Driver::ready()
{
    bool delay_elapsed;
    if (std::chrono::steady_clock::now() <= (last_request+request_delay) ) delay_elapsed = false;
    else
    {
        delay_elapsed = true;
        last_request = std::chrono::steady_clock::now();
    }
    return delay_elapsed;
}




///////////////////////
//USB DRIVER PROCEDURES

int Usb::scan()
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
				
				
				//cout<<"Device "<<++i<<" trovata!"<<endl;
				//cout<<"  Manufacturer: "<<curr_dev->manufacturer_string<<"\nProduct: "<<curr_dev->product_string<<endl;
				
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
			//set_stop(false);	//Resetta il flag
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


/*
int usb::read_show(const unsigned int times, const unsigned int delay)		//uses recv_measure() and displays/uses its result
{
	unsigned int i=0;
	int status=ERROR;
	measure_struct misura;
	
	cout<<"Inizio apertura device..."<<endl;
	hid_device* handle = hid_open(MY_VID, MY_PID, NULL);		//Device in uso (puntatore alla handle del kernel)
	
	if (handle == NULL)
	{
		cout<<"Errore nell'apertura della device!"<<endl;
		status=ERROR;
	}
	else
	{
		cout<<"Periferica aperta!"<<endl;
		
		for(i=0;i<times && status!=ERROR && status!=ABORTED;i++)
		{
			cout<<"| Lettura da "<<handle<<" in corso..."<<endl;
			status=usb::recv_measure(handle,misura);
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
*/

int Usb::recv_measure()	//copies device format data into measure_struct data type
{	
	int bytes_read=0,bytes_to_read=sizeof(measure_struct),i=0,result=ERROR;
	unsigned char buf[bytes_to_read];
	
	if(d==NULL) return ERROR;		//Ritorna ERROR subito se la handle non è valida!
	while(!get_stop() && bytes_read<= bytes_to_read-1 && bytes_read!=-1)	//Questo ciclo si interrompe solo se fermato o se ha letto almeno 6byte
	{
		cout<<"  | Tentativo "<<++i<<endl;
		bytes_read = hid_read_timeout(d,buf,bytes_to_read,5000);
		if (bytes_read < bytes_to_read) cout<<"  | Lettura fallita."<<endl;
		else
		{
			cout<<"  | "<<bytes_read<<" letti: ";
			cout<<(int)buf[0]<<" "<<(int)buf[1]<<" "<<(int)buf[2]<<" "<<(int)buf[3]<<" "<<(int)buf[4]<<" "<<(int)buf[5]<<" "<<endl;
		}
		if (bytes_read == -1)
		{
		    cout<<"  | ERRORE: Periferica non pronta!"<<endl;
		    d=NULL;             //Resets handle
		}
	}
	
	if(get_stop())				//Se è stato fermato verrà ritornato ABORT
	{
		result=ABORTED;
		//set_stop(false);		//Resetta il flag
	}
	else if (bytes_read==bytes_to_read)
	{
		memcpy( (void*) &m, (void*) buf, bytes_to_read);
		result=NICE;			//Per sicurezza, solo se ha letto esattamente 6byte ritorna NICE
	}
	
	return result;				//Qui ERROR è ritornato solo se avviene un comportamento inaspettato.

}
	
	

short int Usb::request(int type)
{
    

    if(ready()) recv_measure();     //IF request_delay HAS PASSED call recv_measure();
    
    short int measure=0;
    
    switch ( type ) {
    case TEMPERATURE:
      measure=m.temp;
      break;
    case HUMIDITY:
      measure=m.humid;
      break;
    case DUST;
      measure=m.dust;
      break;
    default:
      measure=ERROR;
      break;
    }
        
    return measure;
    
}



/////////////////////////////
//RASPBERRY DRIVER PROCEDURES

int Raspberry::recv_measure()
{
    
    //???
    //m.temp? m.humid?
}



short int Raspberry::request(int type)
{
    //IF request_delay HAS PASSED: recv_measure();
    if(ready()) recv_measure();
    
    short int measure=0;
    
    switch ( type ) {
    case TEMPERATURE:
      measure=m.temp;
      break;
    case HUMIDITY:
      measure=m.humid;
      break;
    default:
      measure=ERROR;
      break;
    }
        
    return measure;
    
}
