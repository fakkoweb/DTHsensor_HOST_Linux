#include "sensors.h"



////////////////////////////
//GENERIC SENSOR PROCEDURES

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

