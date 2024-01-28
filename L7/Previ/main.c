#include <xc.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "config.h"
#include "GLCD.h"
#include "coffeelib.h"
#define _XTAL_FREQ 8000000
#define DELETE 0x08
#define ENTER 0x0D
#define INIY 1
#define INIX 1
#define ENDY 7
#define ENDX 24

const char * s1 = "L7 Projecte\n";
const char * s2 = "Alex Lafuente\n";
const char * s3 = "Gabriel Guillamon\n";

const char * state1 = "Ready\n"; 
const char * state2 = "Running\n";
const char * state3 = "Pouring\n";

int posx = 1; // Posició horitzontal del cursor d'escriptura al GLCD
int posy = 1; // Posició vertical del cursor d'escriptura al GLCD

typedef enum{
    Ready=0,
    Running,
    Pouring
   }state_t;

   state_t estat;

void ShowNames(){
   clearGLCD(0,7,0,127);
   writeTerminal(s1);
   writeTerminal(s2);
   writeTerminal(s3);
   writeTxt(0, 0, s1);
   writeTxt(2, 0, s2);
   writeTxt(4, 0, s3);
}
   
void interrupt high_ISR(){
   if(RC1IF && RC1IE){
      int val;
      char *c[2];
      while(RC1IF){
	 val = RCREG1;
	 c[0] = RCREG1;
      }
      if(val == DELETE || val == ENTER){ // Si s'ha premut tecla de borrar o intro
	 c[1] = "\n";
	 if(val == DELETE){	// Borrar
	    if(posx < ENDX) --posx;
	    else posx = ENDX-1; // 25 el tenim com a posició extra només quan tota la pantalla estuigui ocupada, i en cas que s'escrigui, resetejar-la
	    if(posx == 0){
	       if(posy != INIY) posx = ENDX-1;
	       else posx = INIX;
	       if (posy > INIY) --posy;
	    }
	    clearGLCD(posy, posy, posx*5, posx*5 + 5);
	 }
	 else{ // Intro
	    ++posy;
	    if(posy == ENDY){
	       //posy = 6;
	       clearGLCD(0,7,0,127);
	       posy = INIY;
	    }
	    /*else*/ posx = 1;
	 }
      }
      else{ // Qualsevol altra tecla
	 if(posx < ENDX+1){
	    writeTxt(posy, posx, c);
	    ++posx;
	 }
	 if(posx == ENDX && posy < ENDY-1){
	    posx = 1;
	    ++posy;
	 }
	 else if(posx == ENDX && posy == ENDY-1){ // Si estém al últim byte de la pantalla, que es reseteji la pantalla només si s'escriu un carácter més
	    ++posx;
	 }
	 else if(posx == ENDX+1){ // 25 el tenim com a posició extra només quan tota la pantalla estuigui ocupada, i en cas que s'escrigui, resetejar-la
	    clearGLCD(0,7,0,127);
	    posx = INIX+1; // En posx-1 == 1 tindrem el byte escrit en aquesta iteració
	    posy = INIY; 
	    writeTxt(posy, posx-1, c);
	 }
      } 
   }
}

void configPic(){
   ANSELA=0x00; 
   ANSELB=0x00;                  
   ANSELC=0x00;                  
   ANSELD=0x00;        
   TRISCbits.RC7 = 1; // RX1 pin per input
   TRISCbits.RC6 = 1; // TX1 pin per output   
   TRISD=0x00;		   
   TRISB=0x00;
   PORTD=0x00;
   PORTB=0x00;

   RCSTA1= 0x90	;	// Activar serial port i Rx
   TXSTA1 = 0x24; 	// USART Configuration Register Async, High Baud Rate
   BAUDCON1bits.BRG16 = 1; // manual pág. 275
   SPBRG1 = 16; // Set de Baud rate (manual pág. 275)
   PIE1bits.TXIE = 0; // desactivar transmit interrupt
   RC1IE = 1;
   RCIF = 0;
   RC1IP = 1;	// High priority
   
   GIEH = 1;
   IPEN = 1;
   PEIE = 1;
}

void main(){  
   configPic();
   GLCDinit();
   clearGLCD(0,7,0,127);
   setStartLine(0);
   showScreen("splash");
   __delay_ms(1500);
   ShowNames();
   __delay_ms(1500);
   clearGLCD(0,7,0,127);
   estat = Ready;
   state_t estatPre = estat;
   while(1){
      
      
   }
}