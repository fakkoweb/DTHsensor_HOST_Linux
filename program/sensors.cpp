#include "sensors.h"
#include "functions.h"  //for average and variance generic functions



////////////////////////////
//GENERIC SENSOR PROCEDURES

Sensor::Sensor(int sample_rate, int avg_interval, bool enable_autorefresh = true) 
{
    board=NULL;
    last_raw_index=0;
    last_format_index=0;
    buffer_filled=false;
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
    bool thread_must_exit=false;    //JUST A COPY of close_thread (for evaluating it outside the lock)
    do
    {
        access.lock();        
        
        //New sample
        raw_push(sample());
        format_push(convert(raw_top()));
        new_sample.notify_all();
        
        //New statistic
        if(buffer_filled)           //if buffer has filled buffer_lenght samples
        {
            raw_average=average((float*)raw_buffer,buffer_lenght);
            average=average(format_buffer,buffer_lenght);
            raw_variance=variance((float*)raw_buffer,buffer_lenght);
            variance=variance(format_buffer,buffer_lenght);
            new_statistic.notify_all();
        }

        //Close thread if asked
        thread_must_exit=close_thread;
        
        access.unlock();
    
        //If there is a thread looping, then wait "refresh_rate" seconds
        if(autorefresh) p_sleep(refresh_rate*1000);
    
     while(autorefresh && !thread_must_exit);   //If there is no thread autorefreshing, there must be no loop
    
    return;
    
    
}


short int Sensor::get_raw(int index=0)  //index=n of samples ago ---> 0 is last sample
{
    lock_guard<mutex> access(rw);    

    if(!autorefresh) refresh();         //on demand refresh if autorefresh is FALSE
    if(index==0) return raw_top();
    else return raw_pick(index);

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

void Sensor::plug_to(const Driver<measure_struct,short int>& new_board)
{
    unique_lock<mutex> access(rw,std::defer_lock);
    if(new_board!=NULL)
    {
        
        //closes current autosampling thread, if any
        if(r!=NULL)
        {
            access.lock();
            close_thread=true;
            access.unlock();
            
            r->join();
        }
        
        //FORCE reset of all sensor variables with same autorefresh setting
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

