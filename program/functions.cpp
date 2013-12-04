
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
	    cout<<"Outdoor Dust Local feed id: "<< check_not_zero( loaded_params["sensors"]["dust"]["ext"].get("lfid",0).asInt() ) <<endl;
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

int register_device( const string mac_addr )
{
    int esito=ERROR;
    string check_registration;
    string check_new_registration;
    string device_nf("Missing resource");

    //Check if the device already exists:
    http_get("http://crowdsensing.ismb.it/SC/rest/test-apis/devices/"+mac_addr, check_registration);
    cout<<check_registration;
    check_registration= check_registration.substr(0,16);
    
    //Device already registered?
    if (check_registration==device_nf)		//NO
     { 
    	Json::Value reg_device;
       	reg_device["id"]=0;
      	reg_device["username"]="gruppo19";
      	reg_device["raspb_wifi_mac"]=mac_addr;
       	cout<<reg_device;
      	http_post("http://crowdsensing.ismb.it/SC/rest/test-apis/devices/", reg_device.toStyledString(), check_new_registration);
       	//cout<<check_new_registration;
       	
       	//GESTIONE ERRORE HTTP POST? Se fallisce per qualche motivo esito = ERROR, altrimenti esito = NICE
     }
     else					//YES
     {
     	cout<<"Il device è già registrato!";
     	esito = ABORTED;
     }
     
     return esito;
    
}

int post_report( const string mac_addr, const map<int, Sensor*>& sa )
{
        int esito=ERROR;
	string check_new_registration_p;
	statistic_struct s_tmp;
	//geoloc
	Json::Value position;
	position["kind"]="latitude#location";
	position["timestampMs"]="1274057512199";
	position["latitude"]=37.3860517;
	position["longitude"]=-122.0838511;
	position["accuracy"]=5000.0;
	position["height_meters"]=0.0;
    
	Json::Value sensorpost;
	sensorpost["raspb_wifi_mac"]=mac_addr;
	sensorpost["send_timestamp"]=getTimeStamp();
	
	Json::Value sensor_v;
	sensor_v= Json::Value(Json::arrayValue);
	
	Json::Value misure;
    	std::map<int, Sensor*>::const_iterator row;
	for(row=sa.begin(); row!=sa.end(); row++)
	{
		s_tmp = row->second->get_statistic();
	 	misure["value_timestamp"]=row->second->getTimeStamp();
		misure["average_value"]=s_tmp.average;
		misure["local_feed_id"]=row->first;
		misure["variance"]=s_tmp.variance;
		misure["units_of_measurement"]=row->second->sunits();
		sensor_v.append(misure);
	}


    	Json::Value reg_post= sensorpost;
	reg_post["position"]=position;
	reg_post["sensor_values"]=sensor_v;

	http_post("http://crowdsensing.ismb.it/SC/rest/test-apis/device/"+mac_addr+"/posts", reg_post.toStyledString(), check_new_registration_p);

	//GESTIONE ERRORE HTTP POST? Se fallisce per qualche motivo esito = ERROR, altrimenti esito = NICE
	
	return esito;

}

string getTimeStamp(){

	time_t now;
    	time(&now);
    	char buf[sizeof "2011-10-08T07:07:09Z"];
    	strftime(buf, sizeof buf, "%FT%TZ", gmtime(&now));
 	stringstream ss;
 	string s;
 	ss<<buf;
 	ss>>s;
 	return s;
}


//Chiede al server la lista dei sensori attualmente registrati. Provvede a registrare quelli nuovi se necessario.
//La funzione ritorna:
//- ABORTED se tutti i nuovi sensori erano già registrati,
//- NICE se almeno un nuovo sensore è stato registrato perché non presente,
//- ERROR se almeno un sensore DOVEVA essere registrato ma la registrazione è fallita.
int register_sensors( const string mac_addr, const Json::Value& sd)
{
	int esito=ERROR;
	string registered_sensors_s;

	// check the list of registered sensors:
   	http_get("http://crowdsensing.ismb.it/SC/rest/test-apis/devices/"+mac_addr+"/feeds", registered_sensors_s);
   	//cout<<registered_sensors_s<<endl;
	
	esito = register_sensor(mac_addr,sd,registered_sensors_s);
	return esito;
}


//Controlla l'esistenza di lfid (uno solo!) nel nodo json "sd" passato e in quelli in esso contenuti. Se non presenti in "rs" li registra usando "mac_addr".
//La funzione ritorna:
//- ABORTED se tutti i nuovi sensori erano già registrati,
//- NICE se almeno un nuovo sensore è stato registrato perché non presente,
//- ERROR se almeno un sensore DOVEVA essere registrato ma la registrazione è fallita.
int register_sensor( const string mac_addr, const Json::Value& node, const string rs )
{
	int esito=ABORTED, inner_esito=ERROR;
    	string check_feed;
    	string check_new_registration_s;

	//if you find an lfid code in the node..
    	if(node.isMember("lfid"))
    	{
		//check if the sensor already exists:
		check_feed="\"local_feed_id\":"+node.get("lfid",0).asString();
		if (rs.find(check_feed) == std::string::npos)		//NO
    		{
			Json::Value reg_sensor;
      			reg_sensor["feed_id"]=0;
      			reg_sensor["tags"]=node.get("tags",0).asString();
      			reg_sensor["local_feed_id"]=node.get("lfid",0).asInt();
			reg_sensor["raspb_wifi_mac"]=mac_addr;
			http_post("http://crowdsensing.ismb.it/SC/rest/test-apis/devices/"+mac_addr+"/feeds", reg_sensor.toStyledString(), check_new_registration_s);
    			//cout<<check_new_registration_s;
    			
    			//GESTIONE ERRORE HTTP POST? Se fallisce per qualche motivo esito = ERROR, altrimenti esito = NICE
		}
		else									//YES
		{
			cout<<"This sensor was already registered...";
			esito=ABORTED;
		}
    	}
    	//otherwise maybe the attributes may contain lfids..
    	else
    	{
	    	for(Json::Value::iterator i = node.begin(); i !=node.end(); ++i)
		{
			Json::Value element = (*i);
			//END CONDITION: if the attribute contains other attributes, recursively call myself
			if(element.size()!=0)
			{
				inner_esito=register_sensor(mac_addr, element, rs);
				//PROPAGAZIONE STATO DI ERRORE
				//esito deve riportare l'errore più grave avvenuto in una delle sottochiamate (inner_esito)
				if(inner_esito!=esito && esito!=ERROR)			//Verifica solo se ci sono stati cambiamenti (se esito era ERROR non ci importa più)
				{
					if(inner_esito != ABORTED) esito = inner_esito;		//se esito era ABORTED, inner_esito avrà riportato un cambiamento: reg. effettuata (NICE) o fallita (ERROR)
												//se esito era NICE, l'algoritmo va comunque bene.
				}
			}
		
		}
	}
	
	//Esito riporterà:
	//- ABORTED se i sensori erano già tutti registrati,
	//- NICE se almeno una registrazione è avvenuta (TUTTE con successo),
	//- ERROR se almeno una registrazione era necessaria MA è fallita.
	return esito;	
	
}


int check_status()
{
	string check_authentication;
	Json::Value authentication;
	Json::Reader reader;
	http_get_auth("http://crowdsensing.ismb.it/SC/rest/apis/auth/gruppo19", check_authentication);
	cout<<check_authentication;
	reader.parse(check_authentication,authentication,false);
	cout<< authentication.toStyledString();
	//if(authentication.get("authenticated",0).asString()=="false")
	//Error Handling
	//return 1;
	//else return 0;
}
















