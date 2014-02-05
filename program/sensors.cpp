#include "sensors.h"
#include "functions.h"  //for average and variance generic functions
#include <math.h>



////////////////////////////
//GENERIC SENSOR PROCEDURES

Sensor::Sensor(const int sample_rate, const int avg_interval, const bool enable_autorefresh) : MeanGuy(false)
{

    //Convert avg_interval in a valid chrono type
    if(avg_interval<=0) avg_delay = std::chrono::duration< int, std::milli >::zero();
    else avg_delay = std::chrono::seconds(avg_interval*60);

    board=NULL;
    raw_measure=0;
    
    //MeanGuy.setMin(?);
    statistic.average=0;
    statistic.variance=0;
    statistic.valid=false;		//to be sure it will be set to true!
    statistic.percentage_validity=0;
    statistic.tot_sample=0;
    
    autorefresh=enable_autorefresh;
    refresh_rate = sample_rate;
    close_thread=false;
    r=NULL;

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
        cerr<<"  S| Chiusura thread embedded richiesta..."<<endl;
        r->join();
        cerr<<"  S| Chiusura thread embedded completata."<<endl;
    }
    
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
        cerr<<"  S| Chiusura thread embedded richiesta..."<<endl;
        r->join();
        cerr<<"  S| Chiusura thread embedded completata."<<endl;
    }
    
    board=NULL;
    raw_measure=0;
    
    //MeanGuy.setMin(?);
    statistic.average=0;
    statistic.variance=0;
    statistic.valid=false;		//to be sure it will be set to true!
    statistic.percentage_validity=0;
    statistic.tot_sample=0;
    
    close_thread=false;
    r=NULL;
    
    
    cerr<<"  S| Buffer e variabili resettate."<<endl;
    
    cerr<<"  S| Buffer rigenerati."<<endl;
    
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
		
		//Is thread been asked to be closed?
		thread_must_exit=close_thread;		
        }
        
        if(!thread_must_exit)
        {
		//New sample
		cerr<<" S| Nuova misura di "<<stype()<<" richiesta al driver ("<<(size_t)board<<")"<<endl;
		MeanGuy.addSample();
		raw_measure=sample();
		
		//Conversion
		//OLD:	Sensor does not store the converted measure since it is not needed anymore.
		//	When user asks for a converted measure, it will be converted on-the-go from raw_measure;
		
		//Mean and Variance

		if(raw_measure!=INVALID)
		{
			MeanGuy.add(convert(raw_measure));	//asdfg
			cerr<<" S| Richiesta misura di "<<stype()<<" soddisfatta."<<endl;
		}
		

		//Notify that a new sample is now available
		new_sample.notify_all();

		//Check if avg_interval minutes (alias "chrono::seconds avg_delay") have passed since last Mean&Variance request
		if ( std::chrono::steady_clock::now() > (last_avg_request + avg_delay) )
		{
			//Ask MeanGuy for latest Mean and Variance
			cerr<<" S| Media pronta e richiesta!"<<endl;
			statistic.average=MeanGuy.getMean();	//asdfg
			statistic.variance=MeanGuy.getVariance();	//asdfg
			statistic.tot_sample=MeanGuy.getGlobalSampleNumber();
			if (statistic.tot_sample>0) statistic.percentage_validity = (MeanGuy.getSampleNumber()*100)/statistic.tot_sample;
			else statistic.percentage_validity = 0;
			
			if(statistic.percentage_validity>THRESHOLD)
			{
				statistic.valid=true;
			}
			else
			{
				statistic.valid=false;
			}
			
			//Reset MeanGuy
			MeanGuy.reset();		//asdfg
			
			//Notify that new statistics are now available
			new_statistic.notify_all();
			
			//State that new last request was NOW
			last_avg_request = std::chrono::steady_clock::now();

		}

	}

	if(autorefresh)
	{
		access.unlock();
		
		//Thread should wait "refresh_rate" milliseconds if is not closing
		if(!thread_must_exit) 
			 std::this_thread::sleep_for(std::chrono::milliseconds(refresh_rate));
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



uint16_t Sensor::get_raw()
{   
	uint16_t measure=0;
    	lock_guard<mutex> access(rw);
	if(board!=NULL)
	{
	    if(!autorefresh)
	    {
	    	refresh();         //on demand refresh if autorefresh is FALSE
	    }
	    measure=raw_measure;
	}
	else cerr<<" S| Attenzione: nessuna device allacciata al sensore.\n | Usare il metodo plug_to() per associare."<<endl;
	return measure;
}

void Sensor::plug_to(const Driver<measure_struct,uint16_t>& new_board)
{

	//Reset board and also the thread if present
        if(board!=NULL) reset();
        
        //Set new board
        board = const_cast< Driver<measure_struct,uint16_t>* > (&new_board);
        
        
        //---------------------------------------------------------------
        //Set new last_avg_request initialization: is done only when driver is first attached!! So that:
        // - if autorefresh is true, thread will start calculating on-line average and will reset it when avg_delay has passed since NOW
        // - if autorefresh is false, the average can still make sense if get_measure() and get_raw_measure() are called periodically bu user
       	last_avg_request = std::chrono::steady_clock::now();
        
        
        //Start a new autosampling thread
        if(autorefresh==true)
        {
            	//CALL OF REFRESH THREAD - Avvia il thread per l'autosampling
        	r= new thread (&Sensor::refresh,this);		// Per eseguire refresh() è richiesto this quando è un metodo della classe stessa
		//wait_new_sample();     			// Aspettiamo che il thread abbia almeno calcolato il primo sample() per considerare il sensore "collegato"   	
        }
        
    
}

