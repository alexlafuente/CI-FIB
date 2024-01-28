/* Main.c 
 * Processor: PIC18F45K22
 * Compiler:  MPLAB XC8
 */

#include <stdio.h>
#include <xc.h>
#include <string.h>
#include "config.h"
#include "GLCD.h"
#define _XTAL_FREQ 8000000  

const char * s1 = "L4A GLCD\n";
const char * s2 = "----------\n";
const char * s3 = "ALEX LAFUENTE\n";
const char * s4 = "GABRIEL GUILLAMON\n";
int prev1 = 0;
int prev2 = 0;

void writeTxt(byte page, byte y, char * s) {
   int i=0;
   while (*s!='\n' && *s!='\0') 
   {
      putchGLCD(page, y+i, *(s++));
      i++;
   };
}	

void escriuPos(int x, int y){
   char coord[256];
   ClearDot(prev1, prev2);
   SetDot(x, y);
   prev1 = x;
   prev2 = y;
   sprintf(coord,"(%d,%d)\n",y,32-x);
   writeTxt(0, 0, coord);
}

void main(void)
{
   ANSELA=0x00; 
   ANSELB=0x00;                  
   ANSELC=0x00;                  
   ANSELD=0x00;                  
   
   TRISA = 0x0F;
   TRISD=0x00;		   
   TRISB=0x00;
    
   PORTA = 0x00;
   PORTD=0x00;
   PORTB=0x00;  
   
   GLCDinit();		   //Inicialitzem la pantalla
   clearGLCD(0,7,0,127);   //Esborrem pantalla
   setStartLine(0);        //Definim linia d'inici

   int up1 = 1;
   int up2 = 1;
   int x = 32;
   int y = 0;
   writeTxt(1, 9, s1);
   writeTxt(2, 8, s2);
   writeTxt(3, 2, s3);
   writeTxt(4, 2, s4);
   __delay_ms(1500);
   clearGLCD(0,7,0,127);   //Esborrem pantalla
   escriuPos(x, y);
   
   while (1)
   {   
      if(!PORTAbits.RA0 && !up1){
	 for(x=32; x < 50; ++x)
	 {
	 escriuPos(x, y);
	 __delay_ms(50);
	 }
	 for(x=50; x > 32; --x)
	 {
	 escriuPos(x, y);
	 __delay_ms(50);
	 }
	 escriuPos(x, y); // escriure x última iteració
	 __delay_ms(50);
      }
      if(PORTAbits.RA0 && up1){
	 __delay_ms(2);
	 if(PORTAbits.RA0 && up1){
	    up1 = 0;
	 }
	 else up1 = 1;
      }
      
     if(!PORTAbits.RA1 && !up2){
	 for(x=32; x > 18; --x)
	 {
	 escriuPos(x, y);
	 __delay_ms(50);
	 }
	 for(x=18; x < 32; ++x)
	 {
	 escriuPos(x, y);
	 __delay_ms(50);
	 }
	 escriuPos(x, y); // escriure x última iteració
	 __delay_ms(50);
      }
      if(PORTAbits.RA1 && up2){
	 __delay_ms(2);
	 if(PORTAbits.RA1 && up2){
	    up2 = 0;
	 }
	 else up2 = 1;
      }
      
      
      if(!PORTAbits.RA2){
	 ++y;
	 if(y == 128){
	    y = 0;
	 }
	 __delay_ms(25);
	 escriuPos(x, y);
      }
      
      if(!PORTAbits.RA3){
	 --y;
	 if(y == -1){
	    y = 127;
	 }
	 __delay_ms(25);
	 escriuPos(x, y);
      }
     
   }
}
