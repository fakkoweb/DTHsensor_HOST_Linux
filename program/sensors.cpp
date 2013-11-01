#include "sensors.h"
#include "functions.h"  //for average and variance generic functions



////////////////////////////
//GENERIC SENSOR PROCEDURES

Sensor::Sensor(const int sample_rate, const int avg_interval, const bool enable_autorefresh) 
{
    board=NULL;
    next=0;
    buffer_filled=false;
    raw_average=0;
    average=0;
    raw_variance=0;
    variance=0;
    autorefresh=enable_autorefresh;
    close_thread=false;
    r=NULL;
    
    buffer_lenght = (avg_interval*60)/sample_rate;
    raw_buffer = new short int[buffer_lenght];
    format_buffer = new float[buffer_lenght];

    refresh_rate = sample_rate;
}


Sensor::~Sensor()
{
    unique_lock<mutex> access(rw,std::defer_lock);
    
    //closes current autosampling thread, if any
    if(r!=NULL)
    {
        access.lock();
        close_thread=true;
        access.unlock();
        cout<<"  S| Chiusura thread embedded richiesta..."<<endl;     
        r->join();
        cout<<"  S| Chiusura thread embedded completata."<<endl;
    }
    
    delete raw_buffer;
    delete format_buffer;
}


//Like calling destructor + constructor without creating a new object
void Sensor::reset()
{
    unique_lock<mutex> access(rw,std::defer_lock);
    
    cout<<" | Sto resettando il sensore..."<<endl;
    
    //closes current autosampling thread, if any
    if(r!=NULL)
    {
        access.lock();
        close_thread=true;
        access.unlock();
        cout<<"  S| Chiusura thread embedded richiesta..."<<endl;
        r->join();
        cout<<"  S| Chiusura thread embedded completata."<<endl;
    }
    
    delete raw_buffer;
    delete format_buffer;
    
    board=NULL;
    next=0;
    buffer_filled=false;
    raw_average=0;
    average=0;
    raw_variance=0;
    variance=0;
    close_thread=false;
    r=NULL;
    
    cout<<"  S| Buffer e variabili resettate."<<endl;
    
    raw_buffer = new short int[buffer_lenght];
    format_buffer = new float[buffer_lenght];
    
    cout<<"  S| Buffer rigenerati."<<endl;
    
    cout<<" | Reset del sensore completato."<<endl;
}

void Sensor::refresh()		//This function is called manually or automatically, in which case all sampling operation must be ATOMICAL 
{
    unique_lock<mutex> access(rw,std::defer_lock);
    bool thread_must_exit=false;    	//JUST A COPY of close_thread (for evaluating it outside the lock)
    do
    {
        if(autorefresh)
        {	
        	access.lock();		//ALL sampling operation by thread should be ATOMICAL. So we put locks here (just for autorefreshing thread)  
        				//and not put locks on elementary operations on buffers (it would cause ricursive locks!)
		//cout<<"Thread is alive!"<<endl;
		
		//Thread should now be closed?
		thread_must_exit=close_thread;		
        }
        
        if(!thread_must_exit)
        {
		//New sample
		cout<<" S| Nuova misura di "<<stype()<<" richiesta al driver ("<<(int)board<<")"<<endl;
		push( sample() );
		cout<<" S| Richiesta misura di "<<stype()<<" soddisfatta."<<endl;
		new_sample.notify_all();
		
		//New statistic
		if(buffer_filled)           //if buffer has filled buffer_lenght samples
		{
		    raw_average=faverage((float*)raw_buffer,buffer_lenght);
		    average=faverage(format_buffer,buffer_lenght);
		    raw_variance=fvariance((float*)raw_buffer,buffer_lenght);
		    variance=fvariance(format_buffer,buffer_lenght);
		    new_statistic.notify_all();
		}
	}

	if(autorefresh)
	{
		access.unlock();
		
		//Thread should wait "refresh_rate" seconds if not closing
		if(!thread_must_exit) 
			p_sleep(refresh_rate*1000);
    	}
    
    }while(autorefresh && !thread_must_exit);   //If there is no thread autorefreshing, there must be no loop
    
    return;
    
    
}

void Sensor::wait_new_sample()
{
    unique_lock<mutex> access(rw);
    if(autorefresh) new_sample.wait(access);
}

void Sensor::wait_new_statistic()
{
    unique_lock<mutex> access(rw);
    if(autorefresh) new_statistic.wait(access);
}



short int Sensor::get_raw(const int index)  //index=n of samples ago ---> 0 is last sample
{   
	short int measure=0;
    	lock_guard<mutex> access(rw);
	if(board!=NULL)
	{
	    if(!autorefresh)
	    {
	    	refresh();         //on demand refresh if autorefresh is FALSE
	    }
	    if(index==0) measure=raw_top();
	    else measure=raw_pick(index);
	}
	else cout<<" S| Attenzione: nessuna device allacciata al sensore.\n | Usare il metodo plug_to() per associare."<<endl;
	return measure;
}

void Sensor::plug_to(const Driver<measure_struct,short int>& new_board)
{
    //if(new_board!=NULL)
    //{
        if(board!=NULL) reset();
        
        //Set new board
        board = const_cast< Driver<measure_struct,short int>* > (&new_board);
        
        //Start a new autosampling thread
        if(autorefresh==true)
        {
            	//CALL OF REFRESH THREAD - Avvia il thread per l'autosampling
        	r= new thread (&Sensor::refresh,this);	// Per eseguire refresh() è richiesto this quando è un metodo della classe stessa
		wait_new_sample();     			// Aspettiamo che il thread abbia almeno calcolato il primo sample() per considerare il sensore "collegato"   	
        }
    //}
    
}

