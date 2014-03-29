#include "sensors.h"
#include "functions.h"  //for average and variance generic functions
#include <math.h>



////////////////////////////
//GENERIC SENSOR PROCEDURES

Sensor::Sensor(const int sample_rate, const int avg_interval, const bool enable_autorefresh, const bool enable_mean_offset) : MeanGuy(enable_mean_offset)
{

    //Convert avg_interval in a valid chrono type
    if(avg_interval<=0) avg_delay = std::chrono::duration< int, std::milli >::zero();
    else avg_delay = std::chrono::seconds(avg_interval*60);

    board=NULL;
    raw_measure=0;
    
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
    int num_total_samples=0;		//Number of total samples (both VALID and INVALID) got from driver

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
        	//Notice about Raw measure Conversion
        	//	Sensor does not store the converted measure since a specific "convert()" pure virtual
        	//	method is used on-the-go, whenever real measure is needed:
		//	- when user asks for a converted measure (see "get_measure()")
		//	- when measure is passed to MeanGuy for mean and variance calculation (see below)
        	//	EACH SUBCLASS EXTENDING SENSOR MUST IMPLEMENT ITS VERSION OF "convert()"!!
        
        
		//NEW SAMPLE & validity check
		cerr<<" S| Nuova misura di "<<stype()<<" richiesta al driver ("<<(size_t)board<<")"<<endl;
		raw_measure=sample();	//request new sample from driver
		if(raw_measure!=INVALID)	//ONLY IF MEASURE IS VALID..
		{
			MeanGuy.add(convert(raw_measure));	//..give MeanGuy next converted measure for on-line calculation
			cerr<<" S| Richiesta misura di "<<stype()<<" soddisfatta."<<endl;
		}
		num_total_samples++;	//then increment COUNT of TOTAL samples
		
		
		//Notify that a new sample is now available
		new_sample.notify_all();


		//MEAN AND VARIANCE
		//Check if avg_interval minutes (alias "chrono::seconds avg_delay") have passed since last Mean&Variance request
		if ( std::chrono::steady_clock::now() > (last_avg_request + avg_delay) )
		{
			//Assign to statistic latest Mean and Variance calculated by MeanGuy
			cerr<<" S| Media pronta e richiesta!"<<endl;
			statistic.average=MeanGuy.getMean();
			statistic.variance=MeanGuy.getVariance();
			//Assign to statistic also number of total samples, valid and invalid, taken
			statistic.tot_sample=num_total_samples;
			//Estimate STATISTIC VALIDIY, comparing number of TOTAL samples with number of VALID samples used by MeanGuy
			if (statistic.tot_sample>0) statistic.percentage_validity = (MeanGuy.getSampleNumber()*100)/statistic.tot_sample;
			else statistic.percentage_validity = 0;
			//Finally declare how to consider this statistic, in respect to a tolerance threshol
			if(statistic.percentage_validity>THRESHOLD)
			{
				statistic.valid=true;
			}
			else
			{
				statistic.valid=false;
			}
			
			//Reset MeanGuy and number of total samples
			MeanGuy.reset();
			num_total_samples=0;
			
			//Notify that new statistics are now available
			new_statistic.notify_all();
			
			//FORCE LOGIC TIME SYNC:
			//Since real parallelism is not always possible and therefore not guaranteed, it is bad to set time on our own
			//THIS:
			//
			//	last_avg_request = std::chrono::steady_clock::now();
			//
			//IS WRONG since it would set a new time in this instant: now() WON'T BE PRECISELY last_avg_request + avg_delay (what user wants), BUT MORE (SO overhead)!!
			//Since small delays sum on time, it is better to state that new last request happened at time last_avg_request + avg_delay:
			last_avg_request = last_avg_request + avg_delay;
			//this way, the sensor is FORCED to be ALWAYS ON TIME in respect to start_time set in PLUGGING PHASE, MINIMIZING DELAY!!

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

void Sensor::plug_to(const Driver<measure_struct,uint16_t>& new_board, const std::chrono::steady_clock::time_point& start_time)
{

	//Reset board and also the thread if present
        if(board!=NULL) reset();
        
        //Set new board
        board = const_cast< Driver<measure_struct,uint16_t>* > (&new_board);
        
        
        //---------------------------------------------------------------
        //Set new last_avg_request initialization: is done only when driver is first attached!! So that:
        // - if autorefresh is true, thread will start calculating on-line average and WILL RESET IT as soon as "avg_delay" minutes have passed since start_time
        // - if autorefresh is false, average is still calculated but it makes sense only if get_measure() or get_raw_measure() are called periodically by user
       	last_avg_request = start_time;
        
        
        //Start a new autosampling thread
        if(autorefresh==true)
        {
            	//CALL OF REFRESH THREAD - Avvia il thread per l'autosampling
        	r= new thread (&Sensor::refresh,this);		// Per eseguire refresh() è richiesto this quando è un metodo della classe stessa
		//wait_new_sample();     			// Aspettiamo che il thread abbia almeno calcolato il primo sample() per considerare il sensore "collegato"   	
        }
        
    
}

