
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

    
    return raw*4;
    
    
}




