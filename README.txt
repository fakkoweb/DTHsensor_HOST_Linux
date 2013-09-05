PROGETTO
------------------
Il progetto usa le librerie (da installare nel sistema):
1- libusb-1.0
2- libcurl-7.32
e contiene le librerie (incluse nella distro)
3- hidapi (wrapper delle libusb per HID)
4- jsoncpp


STRUTTURA DEI FILE
------------------

makefile è il file che contiene le direttive per la compilazione (flag, librerie ecc.)
main.cpp è il file che implementa il progetto
functions.cpp implementa le diverse funzioni del progetto
/control contiene la classe control per la gestione intelligente delle funzioni
	control.cpp la sua implementazione // ATTENZIONE: funziona solo con UNIX
	control.h la sua interfaccia, da includere in qualsiasi file utilizzi le funzioni del progetto
/hidapi contiene la libreria linkata per la gestione di operazioni HID
	hidapi.h da includere in qualsiasi file interagisca direttamente con USB tramite HID
	>>gli altri file sono le librerie che implementano lo stack USB
/config contiene le define per la configurazione di variabili di progetto
	config.h contiene VID, PID, la definizione dei flag di libreria e delle funzioni


PER COMPILARE
-------------

1. Navigare nella cartella del progetto da terminale
2. Digitare "make"
	- make ricompila SOLO i file realmente modificati
	- Per forzare la ricompilazione di TUTTI i file, digitare "make clean" e poi "make"
3. La compilazione può portare warning dovuti a variabili dichiarate ma non usate


SE NECESSARIO, LE LIBRERIE INCLUSE NEI SORGENTI (messe tra "virgolette") ED EVENTUALI FLAG VANNO SETTATI NEL MAKEFILE
Se ad esempio volessimo creare un nuovo file .h da includere nel nostro usb_host.cpp basta scrivere:
# include "nomefile.h"
ed esplicitare il suo percorso nel makefile tra i flag di include (-Ipercorso).
NON bisogna scrivere il percorso nell'include.

