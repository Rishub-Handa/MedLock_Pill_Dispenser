#ifndef CONNECTION_H 
#define CONNECTION_H 

#include "Particle.h"
#include "softap_http.h"

struct Page
{
    const char* url;
    const char* mime_type;
    const char* data;
};

void myPage(const char* url, ResponseCallback* cb, void* cbArg, Reader* body, Writer* result, void* reserved); 
void setUpInternet(); 
bool connectToInternet(); 

#endif