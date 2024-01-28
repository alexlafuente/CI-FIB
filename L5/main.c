   #include <xc.h>
   #include <string.h>
   #include <stdio.h>
   #include <math.h>
   #include "config.h"
   #include "GLCD.h"
   #include "coffeelib.h"
   #define _XTAL_FREQ 8000000

   const char * s1 = "L5 INDIVIDUAL\n";
   const char * s2 = "ALEX LAFUENTE\n";
   const char * lin = "-------------\n";
   const char * s3 = "+\n";
   const char * s4 = "-\n";
   char * coffee = "Coffee is ready!\n";
   char f2 = 0;
   int seconds = 0;
   int dec = 10;
   char barInit = 0;
   char steamState = -1;

   byte barfill = 32;
   byte barEnd = 0;
   byte leftBut = 0;
   byte rightBut = 0;
   char recLeftBut = 0;
   char recRightBut = 0;
   char cancelDelays = 0;

   typedef enum{
    Ready=0,
    Running,
    Pouring
   }state_t;

   state_t estatCrono;

   void updateSeconds(){
      char s[3]; // Per als segons
      sprintf(s, "%ds", seconds);
      writeTxt(5, 119, s);
   }

   void incrementarTmp(){
      if(barfill < 95) {
	 ++seconds;
	 writeByte(6, barfill++, 0xFF);
      }     
      updateSeconds();
      __delay_ms(50);
   }

   void decrementarTmp(){
      char borrar = 0;
      if(seconds == 10) borrar = 1; // Saber si cal borrar 's' extra al contador quan passa de 2 a 1 cifra
      if(barfill > 32){
	 --seconds;
	 writeByte(6, --barfill, 0x01);
      }
      updateSeconds();
      if(borrar) clearGLCD(5, 5, 92, 97);
      if(cancelDelays == 0) __delay_ms(50);   
   }

   void configPic(){
       ANSELA=0x00; 
       ANSELB=0x00;                  
       ANSELC=0x00;                  
       ANSELD=0x00;                  
       TRISA=0x03;
       TRISD=0x00;
       TRISB=0x00;
       PORTD=0x00;
       PORTB=0x00;
   }

   void configTMR(){
       T0CONbits.TMR0ON = 0;
       T0CONbits.T08BIT = 0;
       T0CONbits.T0CS = 0;
       T0CONbits.PSA = 0;
       T0CONbits.T0PS = 1;
       INTCON2bits.TMR0IP = 1;
       IPEN = 1;
       TMR0IF = 0;
       TMR0IE = 0;
       GIEH = 1;
   }

   void configBarra(){
       //Barres horitzontals
       byte aux = barfill - 1;
       for(int i = 0; i < 8; ++i){
	 showImg(6, aux, 0);
	 aux += 8;
       }
       aux = barfill - 1;
       for(int i = 0; i < 8; ++i){
	 showImg(7, aux, 0);
	 aux += 8;
       }
       // Barres verticals
       writeByte(6, barfill - 1, 0xFF);
       writeByte(6, aux, 0xFF); // A la página 6
       writeByte(7, aux, 0x01); // A la página 7
       updateSeconds();
       barInit = 1;
       barEnd = aux;
   }
   
   void configCafe(){
      byte aux = 55;
      showImg(4, aux, 4);
      writeByte(4, aux+8, 0x81);
      writeByte(4, aux+9, 0x81);
      writeByte(4, aux+10, 0xFF);
   }
   
   void showSteam(char state){
      byte aux = 60;
      if(state == 0){
	 writeByte(3, aux-1, 0x7F);
	 writeByte(3, aux+2, 0x7F);
      }
      else if(state == 1){
	 writeByte(3, aux-1, 0x7C);
	 writeByte(3, aux+2, 0x7C);
      }
   }
   
   void configBoto(char pressed, char which){
      if(leftBut == 0) leftBut = barfill - 12;
      if(rightBut == 0) rightBut = barEnd + 4;
      if(!pressed){
	 if(which == 'A' || which == '+'){
	    // Botó +
	    writeByte(6, leftBut-1, 0xC0);
	    showImg(6, leftBut, 1);
	    writeByte(6, leftBut+8, 0xC0);
	 }
	 if(which == 'A' || which == '-'){
	    // Botó -
	    writeByte(6, rightBut-1, 0xC0);
	    showImg(6, rightBut, 1);
	    writeByte(6, rightBut+8, 0xC0);
	 }
      }
      else{
	 if(which == 'A' || which == '+'){
	    // Botó +
	    writeByte(6, leftBut-1, 0xC0);
	    showImg(6, leftBut, 3);
	    writeByte(6, leftBut+8, 0xC0);
	 }
	 if(which == 'A' || which == '-'){
	    // Botó -
	    writeByte(6, rightBut-1, 0xC0);
	    showImg(6, rightBut, 3);
	    writeByte(6, rightBut+8, 0xC0);
	 }
      }
      
   }
   
   void tic(){
      TMR0IF = 0;
      TMR0 = 65535 - 50000;
      if(dec == 10){ // Canviar animació del fum una vegada cada segon
	 ++steamState;
	 if(steamState > 1) steamState = 0;
	 showSteam(steamState);
      }
      --dec;
      ++steamState;
      if(steamState > 1) steamState = 0;
      showSteam(steamState);
      if(estatCrono == Pouring && seconds > 0){
	 if(dec == 0) {
	    decrementarTmp();
	    dec = 10;
	 }
      }
      else if(estatCrono == Pouring && seconds == 0){
	 TMR0IE = 0;
	 T0CONbits.TMR0ON = 0;
	 writeTxt(1, 5, coffee);
	 while(PORTAbits.RA0 && PORTAbits.RA1){ // El café mostra fum i surt el missatge de finalització mentre ambdós botons estiguin premuts
	    __delay_ms(20);
	    ++steamState;
	    if(steamState > 1) steamState = 0;
	    showSteam(steamState);
	 }
	 clearGLCD(1, 1, 25, 102);
	 if(!PORTAbits.RA0) configBoto(0, '+');
	 if(!PORTAbits.RA1) configBoto(0, '-');
	 writeByte(3, 60-1, 0x00);
	 writeByte(3, 60+2, 0x00);
	 estatCrono = Ready;
      }
   }
   
   void interrupt ISR(void){
       if(TMR0IF && TMR0IE){
	   tic();
       }
   }

   void main(){
       configPic();
       GLCDinit();
       clearGLCD(0,7,0,127);
       setStartLine(0);
       writeTxt(2, 5, s1);
       writeTxt(3, 5, lin);
       writeTxt(4, 5, s2);
       __delay_ms(2000);
       clearGLCD(0,7,0,127);
       configBarra();
       configBoto(0,'A');
       writeTxt(5,4, s3); // +
       writeTxt(5, 20, s4); // -
       configCafe();
       configTMR();
       estatCrono = Ready;
       while(1){
	   if(PORTAbits.RA0 && !PORTAbits.RA1){     
	     if(estatCrono == Pouring){
	        configBoto(0, '-');
	        writeByte(3, 60-1, 0x00);
	        writeByte(3, 60+2, 0x00);
	     }
	     estatCrono = Running;
	     cancelDelays = 0;
	     TMR0IE = 0;
	     T0CONbits.TMR0ON = 0;
	     if(!recLeftBut){
	       recLeftBut = 1;
	       configBoto(1, '+');
	     }
	     incrementarTmp();
	   }
	   if(!PORTAbits.RA0 && !PORTAbits.RA1){
	       if(recLeftBut){
		  recLeftBut = 0;
		  configBoto(0, '+');
	       }
	   }
	   if(PORTAbits.RA1 && !PORTAbits.RA0){
	       if(estatCrono == Pouring){
		  configBoto(0, '+');
		  writeByte(3, 60-1, 0x00);
		  writeByte(3, 60+2, 0x00);
	       }
	       estatCrono = Running;
	       cancelDelays = 0;
	       TMR0IE = 0;
	       T0CONbits.TMR0ON = 0;
	       if(!recRightBut){
		  recRightBut = 1;
		  configBoto(1, '-');
	       }
	       decrementarTmp();
	   }
	   if(!PORTAbits.RA1 && !PORTAbits.RA0){
	       if(recRightBut){
		  recRightBut = 0;
		  configBoto(0, '-');
	       }
	   }
	   if (PORTAbits.RA0 && PORTAbits.RA1 && !f2) {
	       __delay_ms(2); // Sist. anti rebots
	       if(PORTAbits.RA0 && PORTAbits.RA1 && !f2){
		  f2 = 1;
		  estatCrono = Pouring;
		  cancelDelays = 1;
		  TMR0 = 65535 - 50000;
		  TMR0IE = 1;
		  T0CONbits.TMR0ON = 1;
		  configBoto(1, 'A');
	       }
	   }
	   else if((!PORTAbits.RA0 || !PORTAbits.RA1) && f2) f2 = 0;
	   if(!PORTAbits.RA0 && !PORTAbits.RA1 && !PORTAbits.RA2){
	       if(estatCrono == Running) estatCrono = Ready;
	   }
	}
   }