/* Main.c 
 * Processor: PIC18F45K22
 * Compiler:  MPLAB XC8
 */

#include <stdio.h>
#include <xc.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "config.h"
#include "GLCD.h"
#define _XTAL_FREQ 8000000

int x = 0;
int y = 0;  
int seed = 0;
int seed2 = 157;

void randomNumber(){
   // Generació de nombre aleatori
      srand(seed);
      x = rand() % 63; // rang del nombre aleatori [0, 63]
      srand(seed2);
      y = rand() % 127;
}

const char * s1 = "L4 GLCD\n";
 int snake[200];
int n = 2;
void writeTxt(byte page, byte y, char * s) {
   int i=0;
   while (*s!='\n' && *s!='\0') 
   {
      putchGLCD(page, y+i, *(s++));
      i++;
   };
}	

void escriuSnake(char dir){
   if(dir == 'd' && snake[1] < 127){
      ClearDot(snake[n-2], snake[n-1]);
      for (int i = n-1; i > 1; i = i-2) {
	snake[i] = snake[i-2];
       }
      for (int i = n-2; i > 0; i = i -2) {
	snake[i] = snake[i-2];
       }    
      ++snake[1]; 
      SetDot(snake[0], snake[1]);
   }
   else if(dir == 'e' && snake[1] > 0){
      ClearDot(snake[n-2], snake[n-1]);
      for (int i = n-1; i > 1; i = i -2) {
	snake[i] = snake[i-2];
       }
      for (int i = n-2; i > 0; i = i -2) {
	snake[i] = snake[i-2];
       }    
      --snake[1]; 
      SetDot(snake[0], snake[1]);      
   }
   
 else if(dir == 'b' && snake[0] < 63){
      ClearDot(snake[n-2], snake[n-1]);
      for (int i = n-1; i > 1; i = i -2) {
	snake[i] = snake[i-2];
       }
      for (int i = n-2; i > 0; i = i -2) {
	snake[i] = snake[i-2];
       }    
      ++snake[0]; 
      SetDot(snake[0], snake[1]);      
   }   
   else if (dir == 'a' && snake[0] > 0) {
    ClearDot(snake[n-2], snake[n-1]);
      for (int i = n-1; i > 1; i = i -2) {
	snake[i] = snake[i-2];
       }
      for (int i = n-2; i > 0; i = i -2) {
	snake[i] = snake[i-2];
       }    
      --snake[0]; 
      SetDot(snake[0], snake[1]);      
   }
   __delay_ms(60);
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
   
   
   writeTxt(1, 9, s1);
  
   __delay_ms(1500);
   clearGLCD(0,7,0,127);   //Esborrem pantalla
   snake[0] = 32;
   snake[1] = 64;
   SetDot(snake[0], snake[1]);
   randomNumber();
   SetDot(x, y);
   while (1)
   {   
      if(!PORTAbits.RA0){
	 __delay_ms(25);
	 escriuSnake('a');
      }
      
     if(!PORTAbits.RA1){
	 __delay_ms(25);
	 escriuSnake('b');
      }
      
      
      if(!PORTAbits.RA2){
	 __delay_ms(25);
	 escriuSnake('d');
      }
      
      if(!PORTAbits.RA3){
	 __delay_ms(25);
	 escriuSnake('e');
      }
      if(snake[0] == x && snake[1] == y){
	 randomNumber();
	 SetDot(x, y);
	 n = n + 2;
      }
      ++seed;
      ++seed2;
   }
}
