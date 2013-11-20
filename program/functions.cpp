
#include "functions.h"




/*

//Curl chiama questa routine (personalizzabile) per ogni "receive" che fa (per header e payload)
//SERVE SE:
//  - è specificata una funzione write_callback tramite l'opzione WRITE_FUNCTION
//  - si può specificare l'ultimo argomento void* tramite opzione WRITE_DATA
//N.B.: SE WRITE_FUNCTION NON E' SPECIFICATO l'opzione WRITE_DATA vuole un FILE*!!
//
//Argomenti:
//ptr -> puntatore ai dati ricevuti
//size -> dimensione della singola unità di informazione
//nmemb -> quante volte size sono stati ricevuti
//stream -> A SCELTA: in questo caso la funzione mette in stream ciò che riceve
//          e ritorna il numero di byte effettivamente scritti!
size_t write_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
	  //Per ogni receive gli chiedo di scrivere sul buffer aperto puntato da stream
      char* c_stream = (char*)stream;                   //create a char version of stream
	  c_stream = new char[(size*nmemb)+1];      
	  strncpy( c_stream, (char*)ptr, size*nmemb ); //to copy bytes I use the void version of stream
	  c_stream[size*nmemb]='\0';                        //to put the \0 I use the char version of stream
	  return (size*nmemb)+1;
}



//HTTP_GET fa una semplice richiesta HTTP all'indirizzo fornitole e restituisce il path del file binario con la risorsa ottenuta
int http_get(const string url, string& json)
{
	  
	  // for each connection (and curl state variables) a handle must be defined  
	  CURL *curl_handle;
	  char* data;
	  char* headers;
	  CURLcode res;
	  
	  // init the curl session   
	  curl_handle = curl_easy_init();
	  
	  
	  if(curl_handle)
	  {
		  // set URL to get   
		  curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str() );
		 
		  // no progress meter please   
		  curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
		 
		  // What operation curl has to do as soon as receives data? Put them in write_data function!   
		  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_callback);
		 

		  // we want the headers be written to this file handle   
		  curl_easy_setopt(curl_handle,   CURLOPT_WRITEHEADER, (void*)headers);
		 
		  // we want the body be written to this file handle instead of stdout   
		  curl_easy_setopt(curl_handle,   CURLOPT_WRITEDATA, (void*)data);
		 
		  // get it!   
		  res=curl_easy_perform(curl_handle);
		  
		  // Check for errors   
		  if(res != CURLE_OK)
		      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		  
		  // convert data to string to export  
		  json.assign(data);    //warning: it uses strlen() so it must be \0 terminated! (see write_callback)
		 
		  // delete bullshit   
		  delete headers;
		  delete data;
	 
		  // cleanup curl stuff   
		  curl_easy_cleanup(curl_handle);
		  
	  }
	 
	  return 0;

}

*/

/* OLD FILE GET
//HTTP_GET fa una semplice richiesta HTTP all'indirizzo fornitole e restituisce il path del file binario con la risorsa ottenuta
int http_get(const string url, string& json)
{
	  
	  // for each connection (and curl state variables) a handle must be defined  
	  CURL *curl_handle;
	  
	  static const char *headerfilename = "head.out";
	  FILE* headerfile;
	  static const char *bodyfilename = "body.out";
	  FILE* bodyfile;
	 
	  CURLcode res;
	 
	  res_filename = bodyfilename;
	 
	  // init the curl session   
	  curl_handle = curl_easy_init();
	  
	  
	  if(curl_handle)
	  {
		  // set URL to get   
		  curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str() );
		 
		  // no progress meter please   
		  curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
		 
		  // What operation curl has to do as soon as receives data? Put them in write_data function!   
		  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_callback);
		 
		  // open the files   
		  headerfile = fopen(headerfilename,"wb");
		  if (headerfile == NULL) {
		    curl_easy_cleanup(curl_handle);
		    return -1;
		  }
		  bodyfile = fopen(bodyfilename,"wb");
		  if (bodyfile == NULL) {
		    curl_easy_cleanup(curl_handle);
		    return -1;
		  }
		 
		  // we want the headers be written to this file handle   
		  curl_easy_setopt(curl_handle,   CURLOPT_WRITEHEADER, headerfile);
		 
		  // we want the body be written to this file handle instead of stdout   
		  curl_easy_setopt(curl_handle,   CURLOPT_WRITEDATA, bodyfile);
		 
		  // get it!   
		  res=curl_easy_perform(curl_handle);
		  
		  // Check for errors   
		  if(res != CURLE_OK)
		      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		  
		  // close the header file   
		  fclose(headerfile);
		 
		  // close the body file   
		  fclose(bodyfile);
	 
		  // cleanup curl stuff   
		  curl_easy_cleanup(curl_handle);
		  
	  }
	 
	  return 0;

}
*/

/*

int http_post(const string url, const string json)
{

	CURL* easyhandle;
	//FILE* json;
	CURLcode res;
    //void* dataptr=NULL;
    
    // Set a list of custom headers  
	struct curl_slist* headers=NULL;
	headers = curl_slist_append(headers, "Content-Type: application/json");

	 // init the curl session   
	 easyhandle = curl_easy_init();

	 if(easyhandle)
	 {

        //c_str() converts a string type to a const char*    
		 // set URL to post   
		 curl_easy_setopt(easyhandle, CURLOPT_URL, url.c_str() );

		 // set binary data to post   
		 curl_easy_setopt(easyhandle, CURLOPT_POSTFIELDS, json.c_str() );

		 // set the size of the postfields data (-1 means "use strlen() to calculate it)    
		 curl_easy_setopt(easyhandle, CURLOPT_POSTFIELDSIZE, -1);

		 // pass our list of custom made headers    
		 curl_easy_setopt(easyhandle, CURLOPT_HTTPHEADER, headers);

		 res=curl_easy_perform(easyhandle); // post away!  
		    
		    // Check for errors   
		    if(res != CURLE_OK)
		      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
			      
		 curl_slist_free_all(headers); // free the header list  

	 }

	 return 0;
	  
}

*/







int register_device( const string mac_addr )
{
    string check_registration;
    string check_new_registration;
    string device_nf("Missing resource");

    //Check if the device already exists:
    http_get("http://crowdsensing.ismb.it/SC/rest/test-apis/devices/"+mac_addr, check_registration);
    cout<<check_registration;
    check_registration= check_registration.substr(0,16);
    if (check_registration==device_nf)
     { 
    	json::Value reg_device;
       	reg_device["id"]=0;
      	reg_device["username"]="gruppo19";
      	reg_device["raspb_wifi_mac"]=mac_addr;
       	cout<<reg_device;
      	http_post("http://crowdsensing.ismb.it/SC/rest/test-apis/devices/", reg_device.toStyledString(), check_new_registration);
       	//cout<<check_new_registration;
     }
     /*else
     {	cout<<"Il device è già registrato!";
       }*/
    
}



int register_sensors( const string mac_addr, const Json::Value& sd)
{
    string check_registration_s;
    string check_feed;
    string check_new_registration_s;
	
	// check the list of registered sensors:
   	http_get("http://crowdsensing.ismb.it/SC/rest/test-apis/devices/"+mac_addr+"/feeds", check_registration_s);
   	//cout<<check_registration_s<<endl;

	//check if the temperature sensor already exists:
    check_feed="\"local_feed_id\":"+sd["temp"]["ext"].get("lfid",0).asInt();
	if (check_registration_s.find(check_feed) == std::string::npos)
    {
    	//cout << "non presente!" << '\n';
	    Json::Value reg_sensor;
      	reg_sensor["feed_id"]=0;
      	reg_sensor["tags"]=sd["temp"]["ext"].get("tags",0).asString();
      	reg_sensor["local_feed_id"]=sd["temp"]["ext"].get("lfid",0).asInt();
      	reg_sensor["raspb_wifi_mac"]=mac_addr;
      	//cout<< reg_sensor;
      	http_post("http://crowdsensing.ismb.it/SC/rest/test-apis/devices/"+mac_addr+"/feeds", reg_sensor.toStyledString(), check_new_registration_s);
      	//cout<<check_new_registration_s;  
	}

	check_feed="\"local_feed_id\":"+sd["temp"]["int"].get("lfid",0).asInt();
	if (check_registration_s.find(check_feed) == std::string::npos)
	{
		//cout << "non presente!" << '\n';
	    Json::Value reg_sensor;
      	reg_sensor["feed_id"]=0;
      	reg_sensor["tags"]=sd["temp"]["int"].get("tags",0).asString();
      	reg_sensor["local_feed_id"]=sd["temp"]["int"].get("lfid",0).asInt();
      	reg_sensor["raspb_wifi_mac"]=mac_addr;
      	//cout<< reg_sensor;
      	http_post("http://crowdsensing.ismb.it/SC/rest/test-apis/devices/"+mac_addr+"/feeds", reg_sensor.toStyledString(), check_new_registration_s);
      	//cout<<check_new_registration_s; 

	//check if the humid sensor already exists:
	check_feed="\"local_feed_id\":"+sd["humid"]["ext"].get("lfid",0).asInt();
	if (check_registration_s.find(check_feed) == std::string::npos)
    {
    	//cout << "non presente!" << '\n';
	    Json::Value reg_sensor;
      	reg_sensor["feed_id"]=0;
      	reg_sensor["tags"]=sd["humid"]["ext"].get("tags",0).asString();
      	reg_sensor["local_feed_id"]=sd["humid"]["ext"].get("lfid",0).asInt();
      	reg_sensor["raspb_wifi_mac"]=mac_addr;
      	//cout<< reg_sensor;
      	http_post("http://crowdsensing.ismb.it/SC/rest/test-apis/devices/"+mac_addr+"/feeds", reg_sensor.toStyledString(), check_new_registration_s);
      	//cout<<check_new_registration_s;  
	}

	check_feed="\"local_feed_id\":"+sd["humid"]["int"].get("lfid",0).asInt();
	if (check_registration_s.find(check_feed) == std::string::npos)
	{
		//cout << "non presente!" << '\n';
	    Json::Value reg_sensor;
      	reg_sensor["feed_id"]=0;
      	reg_sensor["tags"]=sd["humid"]["int"].get("tags",0).asString();
      	reg_sensor["local_feed_id"]=sd["humid"]["int"].get("lfid",0).asInt();
      	reg_sensor["raspb_wifi_mac"]=mac_addr;
      	//cout<< reg_sensor;
      	http_post("http://crowdsensing.ismb.it/SC/rest/test-apis/devices/"+mac_addr+"/feeds", reg_sensor.toStyledString(), check_new_registration_s);
      	//cout<<check_new_registration_s;

 	//check if the dust sensor already exists:
	check_feed="\"local_feed_id\":"+sd["dust"].get("lfid",0).asInt();
	if (check_registration_s.find(check_feed) == std::string::npos)
    {
    	//cout << "non presente!" << '\n';
	    Json::Value reg_sensor;
      	reg_sensor["feed_id"]=0;
      	reg_sensor["tags"]=sd["dust"].get("tags",0).asString();
      	reg_sensor["local_feed_id"]=sd["dust"].get("lfid",0).asInt();
      	reg_sensor["raspb_wifi_mac"]=mac_addr;
      	//cout<< reg_sensor;
      	http_post("http://crowdsensing.ismb.it/SC/rest/test-apis/devices/"+mac_addr+"/feeds", reg_sensor.toStyledString(), check_new_registration_s);
      	//cout<<check_new_registration_s;  
	}
	
		
	/*for(Json::Value::iterator i = sd.begin(); i !=sd.end(); ++i)
	{	
		}*/
    		
}



int post_report( const string mac_addr,const map<int, Sensor*>& sa )
{
    
	string check_new_registration_p;
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
	sensorpost["send_timestamp"]=getTimeStamp();//da implementare!
	
	Json::Value sensor_v;
	sensor_v= Json::Value(Json::arrayValue);
	
	Json::Value misure;
	for(MapType::const_iterator row=sa.begin(); row!=sa.end(); row++)
	{ 
	 	misure["value_timestamp"]=row->second->get_TimeStamp_m();//manca
		misure["average_value"]=row->second->get_raw_average();
		misure["local_feed_id"]=row->first;
		misure["variance"]=row->second->get_raw_variance();
		misure["units_of_measurement"]=row->second->sunits();//?? manca
		sensor_v.append(misure);
	}

    Json::Value reg_post= sensorpost;
	reg_post["position"]=position;
	reg_post["sensor_values"]=sensor_v;

	http_post("http://crowdsensing.ismb.it/SC/rest/test-apis/device/"+mac_addr+"/posts", reg_post.toStyledString(), check_new_registration_p);

}

string getTimeStamp(){}




/* OLD FUNCTION
//Prepare json for report
int send_report( map<string,int> &sa )
{


    std::chrono::time_point now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto millis_string = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    string now_string;
    now_string << time_fmt(local) << system_clock::now() ;
    
    
    time_t     now;
    struct tm  now_format;
    char       now_string[35];
    // Get current time
    time(&now);
    // Format time, "ddd yyyy-mm-dd hh:mm:ss zzz"
    now_format = *localtime(&now);
    strftime(now_string, sizeof(now_string), "%Y-%m-%dT%H:%M:%S.650%Z", &now_format);
    printf("%s\n", buf);
    


	string url;
	Json::Value payload;   // Json works like a tree: an attribute may contain a value or
	                       // a subset of attributes with its values and so on.
	                       // A Json::Value will contain the root of the value tree.
	
	// ATTRIBUTE BUILDING
	// Since Json::Value has implicit constructor for all value types, it is not
	// necessary to explicitly construct the Json::Value object:
	//payload["encoding"] = getCurrentEncoding();
	//payload["indent"]["length"] = getCurrentIndentLength();
	//payload["indent"]["use_space"] = getCurrentIndentUseSpace();

    payload["raspb_wifi_mac"] = "00:22:fb:8f:9d:fe";
    payload["send_timestamp"] = now_string;
    payload["position"]["kind"] = "latitude#location";
    payload["position"]["timestampMs"] = millis_string;
    payload["position"]["latitude"] = 37.3860517;
    payload["position"]["longitude"] = -122.0838511;
    payload["position"]["accuracy"] = 5000;
    payload["position"]["height_meters"] = 0;
    //foreach sensor in sa
    
        payload["sensor_values"][]["value_timestamp"] = now_string;
        payload["sensor_values"][]["average_value"] = sensor.get_raw_average();
        payload["sensor_values"][]["local_feed_id"] = sensor.get_feed_id(); oppure sa lato int
        payload["sensor_values"][]["variance"] = sensor.get_raw_variance();
        payload["sensor_values"][]["units_of_measurement"] = "Watts";
        
        
    
    

  
	cout<<payload;  
	
	Json::FastWriter writer;   // Json::Writer contains functions to "write" a Json::Value
	                           // structure to a string (not human readable)
	string json = writer.write( payload );  //TRASFORM the tree into a valid json string
    

    //Send json via POST method
	http_post(url,json);



    // MORE...
	// You can also use streams.  This will put the contents of any JSON
	// stream at a particular sub-value, if you'd like.
	std::cin >> payload["subtree"];

	// And you can write to a stream, using the StyledWriter automatically.
	std::cout << payload;
    

}

*/
