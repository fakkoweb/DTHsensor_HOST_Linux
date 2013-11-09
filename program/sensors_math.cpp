
#include "sensors.h"



//specified in...
#include "functions.h"
/////////////////////////////
// GENERIC MATH FUNCTIONS

float faverage(float* array, int dim)
{
    return 3;
}
float fvariance(float* array, int dim)
{
    return 3;
}




//////////////////////////////////
// SPECIFIC SENSOR MATH FUNCTIONS

float TempSensor::convert(short int raw)
{
 
    return raw*2;

    
    
    
}

float HumidSensor::convert(short int raw)
{

    return raw*3;
    
    
    
}


float DustSensor::convert(short int raw)
{

	float mgm_dust = 0.0;
	float m = 0.16;
	float q = 0.08;
    float factor_conversion = 0.0039;
    float dust_volt = factor_conversion * raw;

    if (dust_volt > 3.5)
    {
    	mgm_dust = 0.55;
    }else{

    	mgm_dust = ((dust_volt * m)-q);
    }

    return mgm_dust;
    
    
}




