#include <xc.h>
#include <string.h>
#include <stdio.h>
#include "config.h"
#include "GLCD.h"
#include "coffeelib.h"
#define _XTAL_FREQ 8000000

// Duty cycle ratio = 250/(4*(249+1)) = 0.25 --> els bits del numerador (250) es divideixen en dues parts: 1 de 8 bits (CCPR3L) i una altra de 2 bits (CCP3CON<5:4> a la que anomenarem LSBDC)
// Frequency = 1/Period = 1/((249+1)*4*(1/8)*16) = 500Hz

const char * s1 = "Duty Cycle percentage\n";
const char * s2 = "%\n";
   
   
void writeTxt(byte page, byte y, char * s) {
   int i=0;
   while (*s!='\n' && *s!='\0') 
   {
      putchGLCD(page, y+i, *(s++));
      i++;
   };
}  

void configPic(){
   CCPTMRS0bits.C3TSEL = 0x00; // CCP3 amb TMR2
   
   PR2 = 249;
   
   CCP3CONbits.DC3B0 = 0;	// CCP3CON<5:4> = 0b10
   CCP3CONbits.DC3B1 = 1;
   
   CCP3CONbits.CCP3M = 0xC;	// CCP3 en mode PWM
   CCPR3L = 0x3E;		// Part alta del numerador en la operació per a obtenir el DC 0x3E == 62
   
   TMR2IE = 1;
   TMR2 = 0;
   TMR2IF = 0;
   T2CON = 0x06;		// Pre 16 i postscalers desactivats
   
   IPEN = 0;			// Deshabilitar prioritats
   GIEH = 1;
   
   ANSELA=0x00;
   ANSELB=0x00;
   ANSELC=0x00;
   ANSELD=0x00;
   ANSELE=0x00;
   TRISAbits.RA0 = 1;
   TRISAbits.RA1 = 1;
   TRISB=0x00;
   TRISD=0x00;
   TRISEbits.RE0 = 0;
   PORTB=0x00;
   PORTD=0x00;
}
  
void interrupt ISR(void){
    if(TMR2IF && TMR2IE){
	TMR2IF = 0;
	TMR2 = 0;
    }
}

void updateDC(int *x){
   writeNum(5, 12, *x);
   __delay_ms(40);
}

void main(){
   configPic();
   GLCDinit();
   clearGLCD(0,7,0,127);
   setStartLine(0);
   writeTxt(3, 3, s1);
   writeTxt(5, 15, s2);
   int LSBDC = 2;	// (Less significant bit del numerador de la operació per a obtenir el DC). Correspon a CCP3CON<5:4> = 0b10
   int DCact = ((CCPR3L << 2) + LSBDC) / 10; // shift logic 2 posicions a esq. per als 8 bits de major pes en aquesta operacio, i es sumen els bits de menor
				  // pes (LSB) perquè ocupin els 2 bits restants. La divisió entre 10 --> (4 * PR2 + 1) == 1000
				  // Tot i així, es divideix entre deu per fer el percentatge (implica multiplicació per 100 de tota la operació)
   updateDC(&DCact);
   while(1){
      if(PORTAbits.RA0 && DCact < 100){
	 ++LSBDC;
	 if(LSBDC == 4){	// Overflow dels 2 bits d'LSB --> 0b100 (s'ha d'actualitzar la part alta)
	    LSBDC = 0; 	//0b00
	    ++CCPR3L;
	 }
	 DCact = ((CCPR3L << 2) + LSBDC) / 10;
	 updateDC(&DCact);
      }
      else if(PORTAbits.RA1 && DCact > 0){
	 --LSBDC;
	 if(LSBDC == -1){ // Overflow dels 2 bits d'LSB --> 0b1111 (s'ha d'actualitzar la part alta)
	    LSBDC = 3;	// 0b11
	    --CCPR3L;
	 }
	 DCact = ((CCPR3L << 2) + LSBDC) / 10;
	 updateDC(&DCact);
      }
   }
}