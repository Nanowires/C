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
#include <string.h>
#include <stdlib.h>
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


//Sendet das Signal an die hardware Clock
void SendAn(int us) {
		digitalWrite(LED, An);
		delayMicroseconds(us);
} 

//Sendet den Befehl
void SendCmd(int cmd) {
	SendAn(Logik);
	digitalWrite(LED,Aus);
	delayMicroseconds(cmd);
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

void SendEnde(){
	SendEins(1);
}

//Wandelt den Hexadezimalen Befehl in einen Binären
void Umwandeln(int* cmd) {
	int i;
	int dec=cmd[0];
	for (i=0;i<16;i++) {
		cmd[i]= dec % 2;
		dec=dec/2;
	}
	for (i=0;i<16;i++) {
		if (cmd[i]!=0) {
			cmd[i]=(3*Null);
		} else {
			cmd[i]=Null;
		}
	}
}

//Funktion zum Senden eines ganzen Befehlsblocks
void Senden(int* cmd) {
	int i;
	SendAnfang();
	for(i=15;i>-1;i--) {
		SendCmd(cmd[i]);
	}
	SendEnde();
}


int main(int argc, char *argv[])
{
	int i;
	int j;
	int cmd[16];
	
	if (argc>3 || argc<2){
		printf("Ungültige Anzahl an Parametern. Für Hilfe -h als Parameter angeben.\n");
		return 0;
	}
	
	for(i=1;i<argc;i++) {     //Nach Parameter -h ,bzw. h suchen und Hilfe ausgeben
	
		if ((strstr(argv[i], "-h")!=NULL) || (strcmp(argv[i],"h")==0)){
			printf("IR Protokoll auf WiringPi Pin 0.\nNur 2 Parameter verwenden, 1. ist der Befehl, 2. die Anzahl\nder Wiederholungen, z.B. vol+ 5.\n");
			printf("Wenn 2. Parameter leer, dann wir Befehl einmal ausgeführt.\nParameter -List zum anzeigen verfügbarer Befehle.\n");
			return 0;
		}
		if((strstr(argv[i], "-List")!=NULL)){
			printf("power\nvol+\nvol-\nup\ndown\nenter\nprog+\nprog-\npip\n1-10\nsource-gr\nsource-wh\ndual\nswap\nhdmi\n"); //Befehle
			return 0;
		}
	}
	if(argc<3){
		j=1;
	} else {
		 j=atoi(argv[2]);
	}
	if(strcmp(argv[1],"power")==0) {
		cmd[0]=0x40BF;
	}
	else if(strcmp(argv[1],"vol+")==0) {
		cmd[0]=0xE01F;
	} else if(strcmp(argv[1],"vol-")==0) {
		cmd[0]=0xD02F;
	} else if(strcmp(argv[1],"up")==0) {
		cmd[0]=0x06F9;
	} else if(strcmp(argv[1],"down")==0) {
		cmd[0]=0x8679;
	} else if(strcmp(argv[1],"enter")==0) {
		cmd[0]=0x16E9;
	} else if(strcmp(argv[1],"prog+")==0) {
		cmd[0]=0x48B7;
	} else if(strcmp(argv[1],"prog-")==0) {
		cmd[0]=0x08F7;
	} else if(strcmp(argv[1],"1")==0) {
		cmd[0]=0x20DF;
	} else if(strcmp(argv[1],"2")==0) {
		cmd[0]=0xA05F;
	} else if(strcmp(argv[1],"3")==0) {
		cmd[0]=0x609F;
	} else if(strcmp(argv[1],"4")==0) {
		cmd[0]=0x10EF;
	} else if(strcmp(argv[1],"5")==0) {
		cmd[0]=0x906F;
	} else if(strcmp(argv[1],"6")==0) {
		cmd[0]=0x50AF;
	} else if(strcmp(argv[1],"7")==0) {
		cmd[0]=0x30CF;
	} else if(strcmp(argv[1],"8")==0) {
		cmd[0]=0xB04F;
	} else if(strcmp(argv[1],"9")==0) {
		cmd[0]=0x708F;
	} else if(strcmp(argv[1],"0")==0) {
		cmd[0]=0x8877;
	} else if(strcmp(argv[1],"pip")==0) {
		cmd[0]=0x04FB;
	} else if(strcmp(argv[1],"source-gr")==0) {
		cmd[0]=0x24DB;
	} else if(strcmp(argv[1],"source-wh")==0) {
		cmd[0]=0x807F;
	} else if(strcmp(argv[1],"dual")==0) {
		cmd[0]=0x00FF;
	} else if(strcmp(argv[1],"swap")==0) {
		cmd[0]=0x24DB;
	} else if(strcmp(argv[1],"hdmi")==0) {
		cmd[0]=0xD12E;
	} else {
		printf("Ungültiger Parameter. Für Hilfe -h als Parameter angeben.\n");
	}
	Umwandeln(cmd);

	wiringPiSetup();	//Startet WiringPi
	piHiPri(1);	//Erhöht die Priorität des Programms
	pinMode (LED, OUTPUT);
	digitalWrite(LED,Aus);
	
	//Schleife zum Ausgleichen der Fehlerrate
	for(i=0;i<j+1;i++){
	  Senden(cmd);
		delay(70);
	}
	return 0;
}

