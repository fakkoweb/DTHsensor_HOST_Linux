#ifndef _P_SLEEP_H_
#define _P_SLEEP_H_

///////////////////////////////
#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/ipc.h>
    #include <sys/types.h>
    #include <sys/wait.h>
    #include <sys/shm.h>
    #include <unistd.h>
    #include <string.h>
#endif


using namespace std;


void p_sleep(unsigned milliseconds);

///////////////////////////////

#endif
