
#include "functions.h"





//When check_not_zero finds <0, makes true the static variable:
static bool zero_found=false;
//Utility function: checks if at least one of a few values is 0 or less
//Returns the same value.
int check_not_zero(int value)
{
    if(value<=0) zero_found=true;
    return value;
}




Json::Value load_params(const string jsonfile)
{
    //Caricamento parametri utente (da parameters.json)
    Json::Reader reader;  // Oggetto per il parsing: trasforma file.json ---> json::value
    Json::Value loaded_params;
    std::ifstream param_input(jsonfile, std::ifstream::binary); //Nuovo oggetto stream in ingresso (associato al file "filename.json")
    if ( !reader.parse( param_input, loaded_params, true ) )
    {
        // report to the user the failure and their locations in the document.
        std::cout  << "Errore di lettura dal file di configurazione:\n"
                   << reader.getFormattedErrorMessages();
        loaded_params.clear();
    }
    else
    {
    
	    //Visualizza i parametri appena caricati (e verifica che gli interi non siano nulli o minori di 0!)
	    cout<<"Parametri caricati"<<endl;
	    cout<<"-- ID data --"<<endl;
	    cout<<"MY_VID: 0x"<<hex<< check_not_zero( loaded_params["device"].get("MY_VID",0).asInt() ) <<endl;
	    cout<<"MY_PID: 0x"<<hex<< check_not_zero( loaded_params["device"].get("MY_PID",0).asInt() ) <<endl;
	    cout<<"Outdoor Temp Local feed id: "<<dec<< check_not_zero( loaded_params["sensors"]["temp"]["ext"].get("lfid",0).asInt() ) <<endl;
	    cout<<"Outdoor Humid Local feed id: "<< check_not_zero( loaded_params["sensors"]["humid"]["ext"].get("lfid",0).asInt() ) <<endl;
	    cout<<"Outdoor Dust Local feed id: "<< check_not_zero( loaded_params["sensors"]["dust"].get("lfid",0).asInt() ) <<endl;
	    cout<<"Indoor Temp Local feed id: "<< check_not_zero( loaded_params["sensors"]["temp"]["int"].get("lfid",0).asInt() ) <<endl;
	    cout<<"Indoor Humid Local feed id: "<< check_not_zero( loaded_params["sensors"]["humid"]["int"].get("lfid",0).asInt() ) <<endl;
	    cout<<"-- Precision data --"<<endl;
	    cout<<"Temperature sample rate (sec): "<< check_not_zero( loaded_params["sensors"]["temp"].get("REFRESH_RATE",0).asInt() ) <<endl;
	    cout<<"Humidity sample rate (sec): "<< check_not_zero( loaded_params["sensors"]["humid"].get("REFRESH_RATE",0).asInt() ) <<endl;
	    cout<<"Dust sample rate (sec): "<< check_not_zero( loaded_params["sensors"]["dust"].get("REFRESH_RATE",0).asInt() ) <<endl;
	    cout<<"Server report interval (min): "<< check_not_zero( loaded_params["report"].get("INTERVAL",0).asInt() ) <<endl;
	    cout<<"...therefore will work on N°samples = ("<< loaded_params["report"].get("INTERVAL",0).asInt() <<"*60)/sample_rate"<<endl;
	    if(zero_found)
	    {
	    	cout<<"WARNING: Some values from configuration are not valid (0 or less). Params cleared!"<<endl;
	    	loaded_params.clear();
	    }
	    zero_found=false;	//reset check_not_zero() flag
	    
    }
    
    return loaded_params;	//will be empty if loading has failed
    
    
}




int register_device( const int vid, const int pid )
{
    
    
    
}



int register_sensors( const int device_id, const map<int, Sensor*>& sa, const Json::Value& sd)
{
    
    
    
}



int post_report( const map<int, Sensor*>& sa )
{
    
    

}

