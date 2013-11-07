
#include "p_sleep.h"


void p_sleep(unsigned milliseconds)			// p_sleep è compatibile per windows e linux
		{
			#ifdef _WIN32
			Sleep(milliseconds);
			#else
			usleep(milliseconds * 1000); 	// because function takes microseconds
			#endif
		}
		
