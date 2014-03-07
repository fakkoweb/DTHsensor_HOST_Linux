
#include "http_manager.h"

#define ERROR	1
#define NICE	0
#define ABORTED	-1

size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
  int written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}


void *myrealloc(void *ptr, size_t size)
{
  /* There might be a realloc() out there that doesn't like reallocing
     NULL pointers, so we take care of it here */
  if(ptr)
    return realloc(ptr, size);
  else
    return malloc(size);
}

size_t WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)data;

  mem->memory = (char*)myrealloc(mem->memory, mem->size + realsize + 1);
  if (mem->memory) {
    memcpy(&(mem->memory[mem->size]), ptr, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
  }
  return realsize;
}

int http_get(const string url, string& json)
{

    CURL *curl_handle;
    CURLcode res;
    int STATUS=ERROR;

    struct MemoryStruct chunk;

    chunk.memory=NULL; /* we expect realloc(NULL, size) to work */
    chunk.size = 0;    /* no data at this point */


    curl_global_init(CURL_GLOBAL_ALL);

    /* init the curl session */
    curl_handle = curl_easy_init();

    /* set URL to get */
    curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str() );

    /* no progress meter please */
    curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);

    /* send all data to this function  */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

    /* write on memory storage variable  */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

    cerr<<"GET operation...";
    /* get it! */
    res=curl_easy_perform(curl_handle);
    cerr<<" Done! Result was";

    // Check for errors
    if(res != CURLE_OK)
    {
    	STATUS=ERROR;
    	cerr<<" error."<<endl;
    }
    else
    {
	json.assign(chunk.memory);    //warning: it uses strlen() so it must be \0 terminated! (see write_callback)
    	STATUS=NICE;
    	cerr<<" success."<<endl;    	
    }


    /* cleanup curl stuff */
    if(chunk.memory)
    free(chunk.memory);
    curl_easy_cleanup(curl_handle);


    return STATUS;

}

int http_post(const string url, const string json_in, string &json_out)
{

    struct MemoryStruct chunk;
	int STATUS;

    //FILE*LocalFile=fopen("myHTTP_log.txt","w");

    chunk.memory=NULL; /* we expect realloc(NULL, size) to work */
    chunk.size = 0;    /* no data at this point */

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
		 curl_easy_setopt(easyhandle, CURLOPT_POSTFIELDS, json_in.data() );

		 // set the size of the postfields data (-1 means "use strlen() to calculate it)
		 curl_easy_setopt(easyhandle, CURLOPT_POSTFIELDSIZE, json_in.size());

		 // pass our list of custom made headers
		 curl_easy_setopt(easyhandle, CURLOPT_HTTPHEADER, headers);
		
		 //to manage HTTP ERRORS
         curl_easy_setopt(easyhandle, CURLOPT_VERBOSE, 1L);
         curl_easy_setopt(easyhandle,CURLOPT_FAILONERROR,true);

         //curl_easy_setopt(easyhandle, CURLOPT_STDERR, LocalFile );

        /* send all data to this function  */
        curl_easy_setopt(easyhandle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

        /* write on memory storage variable  */
        curl_easy_setopt(easyhandle, CURLOPT_WRITEDATA, (void *)&chunk);


	    cerr<<"POST operation...";
	    /* get it! */
	    res=curl_easy_perform(easyhandle);
	    cerr<<" Done! Result was";

    // Check for errors
    if(res != CURLE_OK)
    {
    	STATUS=ERROR;
    	cerr<<" error."<<endl;
    }
    else
    {
	 json_out.assign(chunk.memory);
    	STATUS=NICE;
    	cerr<<" success."<<endl;    	
    }
         
       // json.out.assign(STATUS);

		 curl_slist_free_all(headers); // free the header list
		
		 if(chunk.memory)
      		free(chunk.memory);

	 }

	 return STATUS;
}

int http_post_auth(const string url, const string json_in, string &json_out)
{


    struct MemoryStruct chunk;
	
    int STATUS;	

    //le credenziali di accesso verranno spostate da qui
    const string username("gruppo19");
    const string password("8s6GTYlm7Y");

    chunk.memory=NULL; /* we expect realloc(NULL, size) to work */
    chunk.size = 0;    /* no data at this point */

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

		 /* set HTTP DIGEST AUTHENTICATION property */

         curl_easy_setopt(easyhandle, CURLOPT_USERPWD, (username+":"+password).c_str());

         curl_easy_setopt(easyhandle, CURLOPT_HTTPAUTH, CURLAUTH_BASIC|CURLAUTH_DIGEST);

         //curl_easy_setopt(easyhandle, CURLOPT_CUSTOMREQUEST, "POST");

		 // set binary data to post
		 curl_easy_setopt(easyhandle, CURLOPT_POSTFIELDS, json_in.data() );

		 // set the size of the postfields data (-1 means "use strlen() to calculate it)
		 curl_easy_setopt(easyhandle, CURLOPT_POSTFIELDSIZE, json_in.size());

		 // pass our list of custom made headers
		 curl_easy_setopt(easyhandle, CURLOPT_HTTPHEADER, headers);
        
         //to manage HTTP ERRORS
         curl_easy_setopt(easyhandle, CURLOPT_VERBOSE, 1L);
         curl_easy_setopt(easyhandle,CURLOPT_FAILONERROR,true);

        /* send all data to this function  */
        curl_easy_setopt(easyhandle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

        /* write on memory storage variable  */
        curl_easy_setopt(easyhandle, CURLOPT_WRITEDATA, (void *)&chunk);


    cerr<<"POST-AUTH operation...";
    /* get it! */
    res=curl_easy_perform(easyhandle);
    cerr<<" Done! Result was";

    // Check for errors
    if(res != CURLE_OK)
    {
    	STATUS=ERROR;
    	cerr<<" error."<<endl;
    }
    else
    {
    	STATUS=NICE;
    	cerr<<" success."<<endl;    	
    }



		 curl_slist_free_all(headers); // free the header list
		
			if(chunk.memory)
       		 free(chunk.memory);
	 }

	 return STATUS;

}

int http_get_auth(const string url, string& json)
{

    int STATUS;

    CURL *curl_handle;
    CURLcode res;
    //le credenziali di accesso verranno spostate da qui
    const string username("gruppo19");
    const string password("8s6GTYlm7Y");

    struct MemoryStruct chunk;

    chunk.memory=NULL; /* we expect realloc(NULL, size) to work */
    chunk.size = 0;    /* no data at this point */


    curl_global_init(CURL_GLOBAL_ALL);

    /* init the curl session */
    curl_handle = curl_easy_init();

    /* set URL to get */
    curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str() );

    /* set HTTP DIGEST AUTHENTICATION property */

    curl_easy_setopt(curl_handle, CURLOPT_USERPWD, (username+":"+password).c_str());

    curl_easy_setopt(curl_handle, CURLOPT_HTTPAUTH, CURLAUTH_BASIC|CURLAUTH_DIGEST);

    /* no progress meter please */
    curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);

    /* send all data to this function  */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

    /* write on memory storage variable  */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

    cerr<<"GET-AUTH operation...";
    /* get it! */
    res=curl_easy_perform(curl_handle);
    cerr<<" Done! Result was";

    // Check for errors
    if(res != CURLE_OK)
    {
    	STATUS=ERROR;
    	cerr<<" error."<<endl;
    }
    else
    {
	json.assign(chunk.memory);    //warning: it uses strlen() so it must be \0 terminated! (see write_callback)
    	STATUS=NICE;
    	cerr<<" success."<<endl;    	
    }


    /* cleanup curl stuff */
    if(chunk.memory)
    free(chunk.memory);
    curl_easy_cleanup(curl_handle);


    return STATUS;

}