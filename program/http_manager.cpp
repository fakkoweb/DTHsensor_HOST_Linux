
#include "http_manager.h"
#include <sstream>




size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
  int written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}


static void *myrealloc(void *ptr, size_t size)
{
  /* There might be a realloc() out there that doesn't like reallocing
     NULL pointers, so we take care of it here */
  if(ptr)
    return realloc(ptr, size);
  else
    return malloc(size);
}

static size_t WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)data;

  mem->memory = (char*)myrealloc(mem->memory, mem->size + realsize + 1);
  if (mem->memory) {
    memcpy(&(mem->memory[mem->size]), ptr, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = '\0';
  }
  return realsize;
}



int http_get(const string url, string& json_out)
{

	// Return status variable
	int STATUS=ERROR;
	    
	// Set the data struct to send
	struct MemoryStruct chunk;
	chunk.memory=NULL; /* we expect realloc(NULL, size) to work */
	chunk.size = 0;    /* no data at this point */

	// Set Curl handle and result
	CURL* easyhandle;
	CURLcode res;

	// init the curl session
	easyhandle = curl_easy_init();

	if(easyhandle)
	{

		/* set URL to get */
		curl_easy_setopt(easyhandle, CURLOPT_URL, url.c_str() );

		/* no progress meter please */
		curl_easy_setopt(easyhandle, CURLOPT_NOPROGRESS, 1L);

		/* send all data to this function  */
		curl_easy_setopt(easyhandle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

		/* write on memory storage variable  */
		curl_easy_setopt(easyhandle, CURLOPT_WRITEDATA, (void *)&chunk);


		cerr<<"GET operation...";
		/* get it! */
		res=curl_easy_perform(easyhandle);
		cerr<<" Done! Result was";

		// Check for errors
		stringstream textconverter;
		if(res != CURLE_OK)
		{
			STATUS=ERROR;
			cerr<<" error."<<endl;
		}
		else
		{
			textconverter << chunk.memory;
			json_out.assign(textconverter.str());	//warning: it uses strlen() so it must be \0 terminated! (see write_callback)
			STATUS=NICE;
			cerr<<" success."<<endl;
		}

	}

	// Deallocation and clean up!
	if(chunk.memory!=NULL) free(chunk.memory);	// free buffer memory
	curl_easy_cleanup(easyhandle);			// free libcurl resources
	
	return STATUS;

}


int http_post(const string url, const string json_in, string &json_out)
{
	// Return status variable
	int STATUS=ERROR;
	    
	// Set the data struct to send
	struct MemoryStruct chunk;
	chunk.memory=NULL; /* we expect realloc(NULL, size) to work */
	chunk.size = 0;    /* no data at this point */

	// Set Curl handle and result
	CURL* easyhandle;
	CURLcode res;

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
			//json_out.assign(chunk.memory);	//warning: it uses strlen() so it must be \0 terminated! (see write_callback)
			STATUS=NICE;
			cerr<<" success."<<endl;
		}

	}

	// Deallocation and clean up!
	if(chunk.memory!=NULL) free(chunk.memory);	// free buffer memory
	curl_slist_free_all(headers); 			// free the custom headers list
	curl_easy_cleanup(easyhandle);			// free libcurl resources
	
	return STATUS;

}


int http_get_auth(const string url, string& json_out)
{

	// Return status variable
	int STATUS=ERROR;
	    
	// Set the data struct to send
	struct MemoryStruct chunk;
	chunk.memory=NULL; /* we expect realloc(NULL, size) to work */
	chunk.size = 0;    /* no data at this point */

	// Set Curl handle and result
	CURL* easyhandle;
	CURLcode res;
	
	// Set Access Credentials
	const string username("gruppo19");
	const string password("8s6GTYlm7Y");

	// init the curl session
	easyhandle = curl_easy_init();

	if(easyhandle)
	{

		/* set URL to get */
		curl_easy_setopt(easyhandle, CURLOPT_URL, url.c_str() );

		/* set HTTP DIGEST AUTHENTICATION property */
		curl_easy_setopt(easyhandle, CURLOPT_USERPWD, (username+":"+password).c_str());
		curl_easy_setopt(easyhandle, CURLOPT_HTTPAUTH, CURLAUTH_BASIC|CURLAUTH_DIGEST);

		/* no progress meter please */
		curl_easy_setopt(easyhandle, CURLOPT_NOPROGRESS, 1L);

		/* send all data to this function  */
		curl_easy_setopt(easyhandle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

		/* write on memory storage variable  */
		curl_easy_setopt(easyhandle, CURLOPT_WRITEDATA, (void *)&chunk);


		cerr<<"GET-AUTH operation...";
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
			json_out.assign(chunk.memory);	//warning: it uses strlen() so it must be \0 terminated! (see write_callback)
			STATUS=NICE;
			cerr<<" success."<<endl;
		}

	}

	// Deallocation and clean up!
	if(chunk.memory!=NULL) free(chunk.memory);	// free buffer memory
	curl_easy_cleanup(easyhandle);			// free libcurl resources
	
	return STATUS;

}


int http_post_auth(const string url, const string json_in, string &json_out)
{

	// Return status variable
	int STATUS=ERROR;
	    
	// Set the data struct to send
	struct MemoryStruct chunk;
	chunk.memory=NULL; /* we expect realloc(NULL, size) to work */
	chunk.size = 0;    /* no data at this point */

	// Set Curl handle and result
	CURL* easyhandle;
	CURLcode res;

	// Set a list of custom headers
	struct curl_slist* headers=NULL;
	headers = curl_slist_append(headers, "Content-Type: application/json");

	// Set Access Credentials
	const string username("gruppo19");
	const string password("8s6GTYlm7Y");
	
	// init the curl session
	easyhandle = curl_easy_init();

	if(easyhandle)
	{

		// c_str() converts a string type to a const char*
		// set URL to post
		curl_easy_setopt(easyhandle, CURLOPT_URL, url.c_str() );
		
		/* set HTTP DIGEST AUTHENTICATION property */
		curl_easy_setopt(easyhandle, CURLOPT_USERPWD, (username+":"+password).c_str());
		curl_easy_setopt(easyhandle, CURLOPT_HTTPAUTH, CURLAUTH_BASIC|CURLAUTH_DIGEST);

		// set binary data to post
		curl_easy_setopt(easyhandle, CURLOPT_POSTFIELDS, json_in.data() );

		// set the size of the postfields data (-1 means "use strlen() to calculate it)
		curl_easy_setopt(easyhandle, CURLOPT_POSTFIELDSIZE, json_in.size());

		// pass our list of custom made headers
		curl_easy_setopt(easyhandle, CURLOPT_HTTPHEADER, headers);
		
		// to manage HTTP ERRORS
		curl_easy_setopt(easyhandle, CURLOPT_VERBOSE, 1L);
		curl_easy_setopt(easyhandle,CURLOPT_FAILONERROR,true);

		// curl_easy_setopt(easyhandle, CURLOPT_STDERR, LocalFile );

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
			//json_out.assign(chunk.memory);
			STATUS=NICE;
			cerr<<" success."<<endl;
		}

	}

	// Deallocation and clean up!
	if(chunk.memory!=NULL) free(chunk.memory);	// free buffer memory
	curl_slist_free_all(headers); 			// free the custom headers list
	curl_easy_cleanup(easyhandle);			// free libcurl resources
	
	return STATUS;

}



