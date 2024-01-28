/* Main.c
 */

#include <xc.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "config.h"
#include "GLCD.h"
#define _XTAL_FREQ 8000000  

typedef enum{
    Manual=0,
    Automatic
   }state_t;

const char * s1 = "Duty Cycle: \n";
const char * s2 = "%\n";
const char * s3 = "RPM: \n";
const char * s4 = "Temp1: \n";
const char * s5 = "Temp2: \n";

long preValue = 0;
int first = 1;			
int overflows = 0;
float rpm = 0;
float dirtyRpm = 0;	
unsigned int temp = 0;
unsigned int temp2 = 0;
state_t estat = Automatic;


void writeTxt(byte page, byte y, char * s) {
   int i=0;
   while (*s!='\n' && *s!='\0') 
   {
      putchGLCD(page, y+i, *(s++));
      i++;
   };
}

void interrupt high_ISR(void){
   if(TMR1IF && TMR1IE){
      TMR1IF = 0;
      TMR1 = 0;				
      if(overflows) ++overflows;
   }
   if(TMR0IF && TMR0IE){
      TMR0 = 15535;
      INTCONbits.TMR0IF = 0;
      rpm = 0.5 * dirtyRpm + 0.5 * rpm;
   }
}

void interrupt low_priority low_ISR(void){
   if(CCP5IF && CCP5IE){
      CCP5IF = 0;
      unsigned int value = CCPR5;				
      if(first){
	 preValue = value;
	 first = 0;
      }
      else{
	 long dif;	// És long pel cas en que la diferència sigui superior a 65536
	 if(overflows == 1){
	   dif = value - preValue;
	 }
	 else{
	   dif = value + (overflows - 1)*65536 - preValue;
	 }
	 preValue = value;
	 long period = (8 * dif * 4)/8;	//PRE * ticks entre voltes * 4/Fosc 
	 dirtyRpm = (60 * pow(10,6))/period;
      }
      overflows = 1;	//cada rotació el comptador d'overflows es reinicia (es seteja a 1)
    }
}

void configPIC(){

   // TMR0 per calcular el rpm filtrat
    INTCONbits.TMR0IF = 0;
    INTCONbits.TMR0IE = 1;
    INTCON2bits.TMR0IP = 1;
    T0CONbits.T08BIT = 0;
    T0CONbits.T0CS = 0;
    T0CONbits.PSA = 0;
    T0CONbits.T0PS2   = 0;
    T0CONbits.T0PS1   = 0;
    T0CONbits.T0PS0   = 1;
    T0CONbits.TMR0ON = 1;
    TMR0IP = 1;
    TMR0 = 15535;

   // CCP3 config
   CCPTMRS0bits.C3TSEL=0x00;	 //CCP3 amb TMR2
   PR2 = 249;
   
   CCP3CONbits.DC3B0 = 0;	// CCP3CON<5:4> = 0b10
   CCP3CONbits.DC3B1 = 1;
   
   CCPR3L=0x3E; 		// Part alta del numerador en la operació per a obtenir el DC 0x3E == 62
   CCP3CONbits.CCP3M=0xC;	// CCP3 en mode PWM
   
   TMR2 = 0;
   T2CON=0x06; 			// Pre 16 i postscalers desactivats
   
   // CCP5 config
   CCPTMRS1bits.C5TSEL = 0x0;	// CCP5 amb TMR1
   CCP5CONbits.CCP5M = 0x5;	// CCP5 capture on rising edge
   
   T1CON = 0x31; // PRE = 8
   TMR1 = 0;
   TMR1GE = 0;
   TMR1IF = 0;
   TMR1IE = 1;
   TMR1IP = 1; 	// Alta prioritat
   
   CCP5IF = 0;
   CCP5IE = 1;
   CCP5IP = 0;	// Baixa prioritat
   
   // AN6 config
   ADCON0bits.CHS = 0x6;	// l'input és AN6
   ADCON1bits.PVCFG = 0; 	// Vref+ = Vdd
   ADCON1bits.NVCFG = 0; 	// Vref- = Ground
   ADCON2bits.ADFM = 1;		// Justificat a la dreta
   ADCON2bits.ADCS = 1; 	// TAD = 8/Fosc = 1 us
   ADCON2bits.ACQT = 0x4;	// TACQ = 8*TAD = 8 us
   ADCON0bits.ADON = 1;
   
   GIEH = 1;
   IPEN = 1;	// Habilitar prioritats
   PEIE = 1;
   
   ANSELA=0x00; 
   ANSELB=0x00;                  
   ANSELC=0x00;                  
   ANSELD=0x00;
   ANSELE=0x02;			//RE1 analògic
   TRISEbits.RE0 = 0;
   TRISEbits.RE1 = 1;
   TRISAbits.RA0 = 1;
   TRISAbits.RA1 = 1;
   TRISC=0x01;
   TRISD=0x00;	   
   TRISB=0x00;
    
   PORTD=0x00;
   PORTB=0x00;
}	

void main(void)
{
   configPIC();
   
   GLCDinit();
   clearGLCD(0,7,0,127);
   setStartLine(0);

   writeTxt(2, 1, s1);
   writeTxt(4, 12, s2);
   writeTxt(2, 17, s3);
   writeTxt(0, 1, s4);
   writeTxt(0, 13, s5);				 
			
   unsigned int duty = ((0.25)*4*25); // igual a 1 * 25 = 25 (25 és duty cycle de referència)
   int preDC = duty;
   int DC = duty * 10;
   CCPR3L = DC >> 2;		// 25*10 >> 2 = 0x3E --> valor de referència de la part baixa (CCPR3L)
   
   writeNum(4, 9, duty); 
   writeNum(4, 17, rpm);
   unsigned int preTemp = temp;
   float preDirtyRPM = preDirtyRPM;
   float preRPM = rpm;
   int f3 = 1;
   int reference = 25;
   int newreference = 25;
   char clearOnce = 1;
   char showFirst = 1;
   int printRPM = 0;
   while (1)
   {   
      if(newreference != reference) reference = newreference;
      if(estat == Manual && clearOnce){
	 clearGLCD(0,0,40,50);
	 clearGLCD(0,0,98,110);
	 if(clearOnce) clearOnce = 0;
      }
      if(PORTAbits.RA0){
	 if(reference < 100){
	    ++reference;
	    newreference = reference;
	    duty = ((reference)*4*25)/100;
	    DC = duty * 10;
	    CCPR3L = DC >> 2;
	 }
	 __delay_ms(200);
      }
     if(PORTAbits.RA1) {
	 if(reference > 0) {
	    --reference;
	    newreference = reference;
	    duty = ((reference)*4*25)/100;
	    DC = duty * 10;
	    CCPR3L = DC >> 2;
	}
	__delay_ms(200);
      }
      if(PORTCbits.RC0 && f3){
	 f3 = 0;
	 if(estat == Manual) estat = Automatic;
	 else estat = Manual;
	 showFirst = 1;
      }
      else if(!PORTCbits.RC0) f3 = 1;
   
      if(estat == Automatic){
	 ADCON0bits.GODONE = 1;
	 while(ADCON0bits.GODONE); // Espera activa al godone
	 //Primer métode de càlcul de la temperatura 
	 float Vout = (ADRES * 5.00)/1024.00; 
	 temp = (4050 / (log((4700 - (23500/Vout))/-0.0059257))) - 273.2;
	 if(temp <= 50){
	    duty = ((temp * 2)*4*25)/100; // Reutilitzar la fòrmula per al DC emprada anteriorment, de manera que si temp == 50, DC = 100
	    DC = duty * 10;
	    CCPR3L = DC >> 2;
	    newreference = ((100 * duty)/25)/4;	// Per actualitzar el valor de referència del calcul del DC, quan s'habiliti el control amb els botons
	    if(!clearOnce) clearOnce = 1;	// Per borrar temperatures quan s'habiliti el control amb els botons
	 }
	// Segon mètode de càlcul de la temperatura
	float Vout2 = ((ADRES >> 4) * 5.00)/64.00;				//només 6 bits de resolució (perdem 4 bits respecte a l'anterior resolució, per tant, desplacem ADRES 4 pos a la dreta)
	temp2 = (4050 / (log ((4700 - (23500/Vout2))/-0.0059257))) - 273.2;
      }
      if(preDC != duty){
	    preDC = duty;
	    clearGLCD(4,4,50,59);
	    if(!duty) rpm = 0;
	    writeNum(4, 9, duty);
      }
      if(preRPM != rpm){
	   if(estat == Automatic && printRPM % 40 == 0){
	       preRPM = rpm;
	       clearGLCD(4,4,65,120);
	       writeNum(4, 17, rpm);
	   }
	   else if(estat == Manual && printRPM % 5000 == 0){
	       preRPM = rpm;
	       clearGLCD(4,4,65,120);
	       writeNum(4, 17, rpm);
	   }
	   
      }
      if(preDirtyRPM != dirtyRpm){
	   if(estat == Automatic && printRPM % 40 == 0){
	       preDirtyRPM = dirtyRpm;
	       clearGLCD(5,5,65,120);
	       writeNum(5, 17, dirtyRpm);
	   }
	   else if(estat == Manual && printRPM % 5000 == 0){
	       preDirtyRPM = dirtyRpm;
	       clearGLCD(5,5,65,120);
	       writeNum(5, 17, dirtyRpm);
	   }
	   
      }
      if(preTemp != temp || showFirst){
	   preTemp = temp;
	   if(showFirst) showFirst = 0;
	   clearGLCD(0,0,45,50);
	   writeNum(0, 8, temp);
	   clearGLCD(0,0,103,110);
	   writeNum(0, 20, temp2);
      }
      ++printRPM;
   }
}
