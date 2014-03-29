#ifndef _HTTP_MANAGER_H_
#define _HTTP_MANAGER_H_
//////////////////////

//Load user configuration and define variables
#include "config.h"

#include <iostream>
#include <curl/curl.h>
#include "json.h"
#include <string.h>


using namespace std;

struct MemoryStruct {
  char *memory;
  size_t size;
};


size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream);
void *myrealloc(void *ptr, size_t size);
size_t WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data);
int http_get(const string url, string& json_out);
int http_post(const string url, const string json_in, string &json_out);
int http_post_auth(const string url, const string json_in, string &json_out);
int http_get_auth(const string url, string& json_out);





/////////////////
#endif

