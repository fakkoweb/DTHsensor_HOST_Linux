#include "control.h"
#include "curl/curl.h"
#include "json.h"


//Curl chiama questa routine (personalizzabile) per ogni "receive" che fa
size_t write_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
	  //Per ogni receive gli chiedo di scrivere sul file aperto puntato da stream
	  int written = fwrite(ptr, size, nmemb, (FILE *)stream);
	  return written;
}



//HTTP_GET fa una semplice richiesta HTTP all'indirizzo fornitole e restituisce il path del file binario con la risorsa ottenuta
int http_get(const string url, string& res_filename)
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
	 	 
	 	 /*
	 	 json = fopen(json_filename.c_str(),"wb");
		 if (json == NULL) {
		    curl_easy_cleanup(easyhandle);
		    return -1;
		 }
		 */
    
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


int send_report()
{

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
  
	cout<<payload;  
	
	Json::FastWriter writer;   // Json::Writer contains functions to "write" a Json::Value
	                           // structure to a string (not human readable)
	string json = writer.write( payload );  //TRASFORM the tree into a valid json string
    

    //Send json via POST method
	http_post(url,json);



    /* MORE...
	// You can also use streams.  This will put the contents of any JSON
	// stream at a particular sub-value, if you'd like.
	std::cin >> payload["subtree"];

	// And you can write to a stream, using the StyledWriter automatically.
	std::cout << payload;
    */



}

