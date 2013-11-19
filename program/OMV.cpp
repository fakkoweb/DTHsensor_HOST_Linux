/*
 * OMV.cpp
 *
 *  Created on: Nov 19, 2013
 *      Author: gabriele
 */
/*class OMV {
private:
	int n;
	double mean;
	double M2;
	double min;
public:
	OMV();
	virtual ~OMV();
	void reset();
	void add(double x_i,bool min);
	double getMean();
	double getVariance();
	double getMin();
	double setMin();
};*/
#include "OMV.h"

/*
 *  OMV::OMV(bool aMin=false)
 *	Il costruttore prende come variabile opzionale aMin.
 *	se aMin è settato a true la classe si occupa anche di calcora il minimo
 *	fino al prossimo reset;
 *
 */
OMV::OMV(bool aMin) {
	if(aMin){
		minCalc=true;
	}
	reset();
}

OMV::~OMV() {
	// TODO Auto-generated destructor stub
}

/*
 * void OMV::reset(double min=0)
 * il reset ha un argomento opzionale che permette di settare
 * al reset un minimo personalizzato
 */

void OMV::reset(double min) {
	n=0;
	mean=0;
	M2=0;
	if(minCalc) this->min=min;
}

/*
 *	void OMV::add(double x)
 *	la funzione aggiunge un double al calcolo della media online
 */

void OMV::add(double x) {
    n++;
    double delta = x - mean;
    mean = mean + delta/n;
    M2 = M2 + delta*(x - mean);

    if(min && this->min>x) this->min=x;
}

/*
 * 		double OMV::getMean()
 * 	Ritorna la media attuale;
 *
 */

double OMV::getMean() {
	return mean;
}

/*
 *		double OMV::getVariance()
 *	ritorna la varianza attuale
 *
 */

double OMV::getVariance() {
	if (M2>0) return M2/(n - 1);
	else return 0;
}

/*
 * 		double OMV::getMin()
 * 	ritorna il minimo
 *
 */
double OMV::getMin(){
	return min;
}
/*
 * 		void OMV::setMin(double aMin)
 * 	setta il minimo e attiva il calcolo del min
 *
 */
void OMV::setMin(double aMin){
	min=aMin;
	minCalc=true;
}
