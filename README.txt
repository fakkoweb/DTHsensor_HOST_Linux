PROGETTO
------------------
Il progetto usa le librerie (da installare nel sistema):
1- libusb-1.0
2- libcurl-7.32
3- libboost-system e libboost-chrono
e contiene le librerie (incluse nella distro, da non installare)
3- hidapi (wrapper delle libusb per HID)
4- jsoncpp


STRUTTURA DEI FILE
------------------

makefile è il file che contiene le direttive per la compilazione (flag, librerie ecc.)
	- per ogni nuova libreria inclusa, possono essere necessari flag per il compilatore o il linker!
main.cpp è il file che implementa il progetto
parameters.json contiene le costanti softcoded (caricate all'avvio) del progetto
/program contiene le implementazioni delle diverse funzioni del progetto
	- sistemare qui i nuovi .cpp
	- aggiungere nella lista del makefile (COBJS o CPPOBJS) la path del nuovo file ma con estensione .o
/include contiene le intestazioni per l'implementazione del progetto
	- sistemare qui i nuovi .h  --> nessuna modifica richiesta nel makefile!
	- config.h contiene le costanti hardcoded per il progetto
/libs contiene intestazioni e implementazione delle librerie INCLUSE nel progetto (sono ricompilate)


PER COMPILARE
-------------

0. Installare nel sistema le librerie richieste
1. Navigare nella cartella del progetto da terminale
2. Digitare "make"
	- make ricompila SOLO i file realmente modificati
	- Per forzare la ricompilazione di TUTTI i file, digitare "make clean" e poi "make"
3. La compilazione può portare warning dovuti a variabili dichiarate ma non usate fino a che il progetto non sarà concluso



