/*
 * Copyright 2014  <pi@raspberrypi>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */


#include <stdio.h>
#include <wiringPi.h>
#include "piHiPri.h"

#define LED 7
#define Anfang 4400 //2000
#define Anfang_Null 4450
#define Logik 500 //340
#define Eins 1470
#define Null 440
#define An 1
#define Aus 0
#define Clk 12
#define Freq 38000


//Sendet manuell das Clock Signal
void SendAn(int us) {
	int i;
	digitalWrite(LED,An);
	for (i=0;i<(us/26);i++) {
		digitalWrite(LED, An);
		delayMicroseconds(Clk);
		digitalWrite(LED, Aus);
		delayMicroseconds(Clk);
	}
} 

//Sendet eine logische 1
void SendEins(int j) {
	int i;
	for(i=0;i<j;i++){
		SendAn(Logik);
		digitalWrite(LED,Aus);
		delayMicroseconds(Eins);
	}
}

//Sendet eine logische 0
void SendNull(int j) {
	int i;
	for(i=0;i<j;i++) {
		SendAn(Logik);
		digitalWrite(LED,Aus);
		delayMicroseconds(Null);
	}
}

//Sendet das Anfangssignal
void SendAnfang() {
	int j;
	SendAn(Anfang);
	digitalWrite(LED,Aus);
	delayMicroseconds(Anfang_Null);
	for(j=0;j<2;j++) {
		SendEins(3);
		SendNull(5);
	}
}

void SendPower() {
	SendNull(1);
	SendEins(1);
	SendNull(6);
	SendEins(1);
	SendNull(1);
	SendEins(6);
}

void SendEnde(){
	SendEins(1);
}

int main(int argc, char *argv[])
{
	wiringPiSetup();	//Startet WiringPi
	piHiPri(1);	//Erhöht die Priorität des Programms
	pinMode (LED, OUTPUT);
	digitalWrite(LED,Aus);
	int i;
	
	//Schleife zum Ausgleichen der Fehlerrate
	for(i=0;i<10;i++){
		SendAnfang();
		SendPower();
		SendEnde();
		delay(70);
	}
	return 0;
}

