#include "sensors.h"



////////////////////////////
//GENERIC SENSOR PROCEDURES

Sensor::Sensor(int sample_rate, int avg_interval, bool enable_autorefresh = true) 
{
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
    
    buffer_lenght = (avg_interval*60)/sample_rate;
    raw_buffer = new short int[buffer_lenght];
    format_buffer = new short int[buffer_lenght];

    refresh_rate = sample_rate;
}

~Sensor::Sensor()
{
    if(r!=NULL)
    {
        access.lock();
        close_thread=true;
        access.unlock();
        
        r->join();
    }
    
    delete raw_buffer;
    delete format_buffer;
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
        //if buffer has filled buffer_lenght samples
        /*
            raw_average=average((float*)raw_buffer,buffer_lenght);
            average=average(format_buffer,buffer_lenght);
            raw_variance=variance((float*)raw_buffer,buffer_lenght);
            variance=variance(format_buffer,buffer_lenght);
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
    if(index<buffer_lenght && index>=0)
    {
        if(index>last_raw_index) location=buffer_lenght-(index-last_raw_index);
        else location=(last_raw_index-index)%buffer_lenght;
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
        /*
        //closes current autosampling thread, if any
        if(r!=NULL)
        {
            access.lock();
            close_thread=true;
            access.unlock();
            
            r->join();
        }
        
        //FORCE reset of all sensor variables with same autorefresh setting
        Sensor(autorefresh);
        */
        
        ~Sensor();
        Sensor(autorefresh);
        
        //Set new board
        board=new_board;
        
        //Start a new autosampling thread
        if(autorefresh==true)
        {
            //CALL OF REFRESH THREAD - Avvia il thread per l'autosampling
        	r= new thread (&Sensor::refresh,this);	// Per eseguire refresh() è richiesto this quando è un metodo della classe stessa
        }
    }
    
}

