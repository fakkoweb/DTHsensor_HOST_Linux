#ifndef _CONFIG_H_
#define _CONFIG_H_

///////////////////////////////
//Parametri di configurazione//
#define MY_VID 0x04D8
#define MY_PID 0x0013

//Costanti per esprimere lo status delle funzioni
#define RUNNING	1
#define IDLE	0

//Costanti per esprimere l'esito delle funzioni
#define ERROR	-1
#define NICE	0
#define ABORTED	1
/*
#define DUST_UNDEF	21	//Ok with warnings
#define TEMP_UNDEF	22
#define HUMID_UNDEF	23
*/

//Costanti per distinguere i tipi di sensori
#define UNDEF   0
#define TEMP    1
#define HUMID   2
#define DUST    3

//Costanti per il prelievo misure
#define SENSOR_BUFFER   30  //in numero di misure
#define NEW_JSON_SEND   5   //in minuti
#define SENSOR_REFRESH_RATE     5   //in secondi

#define HOST_THREADS		//Commenta per disabilitare il supporto thread (c++11)

///////////////////////////////

#endif
