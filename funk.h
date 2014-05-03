#include <stdio.h>
#include <wiringPi.h>
#include <string.h>
#include <stdlib.h>

#define Trans 3
#define Nullf 1050
#define Einsf 1370
#define Null_Anf 530
#define Eins_ausf 250

void sendNull(int i){
	int j;
	for(j=0;j<i;j++){
		digitalWrite(Trans,An);
		delayMicroseconds(Null_Anf);
		digitalWrite(Trans,Aus);
		delayMicroseconds(Nullf);
	}
}

void sendEins(int i){
	int j;
	for(j=0;j<i;j++){
		digitalWrite(Trans,An);
		delayMicroseconds(Einsf);
		digitalWrite(Trans,Aus);
		delayMicroseconds(Eins_ausf);
		sendNull(1);
	}
}

void an() {
	sendNull(3);
	sendEins(3);
	sendNull(2);
	sendEins(7);
}

void aus() {
	sendNull(3);
	sendEins(3);
	sendNull(2);
	sendEins(6);
	sendNull(2);
}

void funk(char string[])
{
	int j;
	wiringPiSetup();
	pinMode(Trans,OUTPUT);
	digitalWrite(Trans,Aus);
	if(strstr(string,"ON")) {
		for(j=0;j<10;j++){
		an();
		delay(5);
	}
	} else if(strstr(string,"OFF")) {
		for(j=0;j<10;j++){
		aus();
		delay(5);
	}
	}
}
