
#include "control.h"
#include "hidapi.h"
#include "curl/curl.h"
#include "json.h"



///////////////////////
//USB DRIVER FUNCTIONS

static int Usb::scan()
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

static int Usb::recv_measure()	//copies device format data into measure_struct data type
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
	
	

static short int Usb::request(int type)
{
    
    //IF request_delay HAS PASSED: recv_measure();
    
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
      break;
    }
        
    return measure;
    
}



/////////////////////////////
//RASPBERRY DRIVER FUNCTIONS

static int Raspberry::recv_measure()
{
    //???
    //m.temp? m.humid?
}



static short int Raspberry::request(int type)
{
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
      break;
    }
        
    return measure;
    
}





////////////////////////////
//VIRTUAL SENSORS FUNCTIONS

Sensor::Sensor()
{
    refresh_rate=SENSOR_REFRESH_RATE;
    board=NULL;
    last_raw_index=0;
    last_format_index=0;
    raw_average=0;
    average=0;
    raw_variance=0;
    variance=0;
    autorefresh=enable_autorefresh;
    close_thread=false;
    r==NULL;
}

~Sensor::Sensor()
{
    if(r!=NULL) r->join();
}

void Sensor::refresh()
{
    unique_lock<mutex> access(rw,std::defer_lock);
    bool thread_must_exit;          //JUST A COPY of close_thread (for safe evaluation)
    do
    {
        access.lock();        
        raw_push(sample());
        format_push(convert(raw_top()));
        //if buffer has filled SENSOR_BUFFER samples
        /*
            raw_average=average((float*)raw_buffer,SENSOR_BUFFER);
            average=average(format_buffer,SENSOR_BUFFER);
            raw_variance=variance((float*)raw_buffer,SENSOR_BUFFER);
            variance=variance(format_buffer,SENSOR_BUFFER);
        */
        new_sample.notify_all();
        thread_must_exit=close_thread;
        access.unlock();
    }
    
    // if(autorefresh) p_sleep(1000);
        
    }while(autorefresh && !thread_must_exit);
    
    return;
    
    
}


short int Sensor::get_raw(int index=0)  //index=n of samples ago ---> 0 is last sample
{
    lock_guard<mutex> access(rw);    
    int location;
    if(!autorefresh) refresh();         //on demand refresh if autorefresh is FALSE
    if(index<SENSOR_BUFFER && index>=0)
    {
        if(index>last_raw_index) location=SENSOR_BUFFER-(index-last_raw_index);
        else location=(last_raw_index-index)%SENSOR_BUFFER;
        return raw_buffer[location];
    }
    else return 0;
}


void Sensor::wait_new_sample()
{
    unique_lock<mutex> access(rw);
    new_sample.wait(access);
}

void Sensor::wait_new_sample()
{
    unique_lock<mutex> access(rw);
    new_statistic.wait(access);
}

void Sensor::plug_to(const driver_call new_board)
{
    unique_lock<mutex> access(rw,std::defer_lock);
    if(new_board!=NULL)
    {
        if(r!=NULL)
        {
            access.lock();
            close_thread=true;
            access.unlock();
            
            r->join();
            close_thread=false;
        }
        board=new_board;
        if(autorefresh==true)
        {
            //CALL OF REFRESH THREAD - Avvia il thread per l'autosampling
        	r= new thread (&Sensor::refresh,this);	// Per eseguire refresh() è richiesto this quando è un metodo della classe stessa
        }
    }
    
}



float average(float* array, int dim)
{
    
    
    
}

float variance(float* array, int dim)
{
    
    
    
    
}

float TempSensor::convert(short int raw)
{
    //short int raw_top() --> CONVERT --> format_push(float)
    
    
    
    
}

float HumidSensor::convert(short int raw)
{
    //short int raw_top() --> CONVERT --> format_push(float)
    
    
    
    
}


float DustSensor::convert(short int raw)
{
    //short int raw_top() --> CONVERT --> format_push(float)
    
    
    
    
}




