#include <stdio.h>
#include <xc.h>
#include <string.h>
#include "config.h"
#include "GLCD.h"
#define _XTAL_FREQ 8000000 

typedef enum{
    Ready=0,
    Running,
    Stopped
}state_t;

// Global Variables

const char * s1 = "L4b\n";
const char * s2 = "------\n";
const char * s3 = "Alex Lafuente\n";
const char * s4 = "Gabriel Guillamon\n";
unsigned int decimes = 0;
unsigned int segons = 10;
char comptador[200];
char estatReady[200];
char estatRunning[200];
char estatStopped[200];
state_t estatCrono = Ready;
int a = 1;

void writeTxt(byte page, byte y, char * s) {
    int i=0;
    while (*s!='\n' && *s!='\0') 
    {
        putchGLCD(page, y+i, *(s++));
        i++;
    };
}

void updateGLCD(){
    sprintf(comptador, "%d, %d\n", segons, decimes); // comptador conté els segons i les décimes
    writeTxt(3, 10, comptador);
    if(estatCrono == Ready){
        writeTxt(0, 0, estatReady);
    }
    else if(estatCrono == Running && segons == 9 && decimes == 9){ // només una iteració per més eficiència
        writeTxt(0, 0, estatRunning);
    }
    else if(estatCrono == Stopped){
        writeTxt(0, 0, estatStopped);
    }
}

void tic(void) {
    --decimes;
    if(decimes == -1){ // Establir cicle de les dècimes
        decimes = 9;
        --segons;
    }
    else if(segons == 0 && decimes == 0){ // Ha acabat el compte enrere, deshabilitar timer
	estatCrono = Stopped;
	INTCONbits.TMR0IE = 0;
	T0CONbits.TMR0ON = 0;
        clearGLCD(0, 1, 0, 50);
        updateGLCD();
    }
}

void interrupt RSI(void){
    if(INTCONbits.TMR0IF && INTCONbits.TMR0IE){
        TMR0 = 15535;
        INTCONbits.TMR0IF = 0;
        tic();
    }
}

// Falling edge detection and debouncing
char inputDetector() {
    char detected = 0;
    if(!PORTCbits.RC0 && !a){
	 a = 1;
	 detected = 1;
    }
    else if(PORTCbits.RC0 && a){
      __delay_ms(2);
      if(PORTCbits.RC0 && a){
	 a = 0;
	 detected = 0;
      }
    }
    return detected;
}

void configPIC(){
    ANSELB = 0x00;
    ANSELC = 0x00;
    ANSELD = 0x00;
    
    TRISB = 0x00;
    TRISC = 0x01;
    TRISD = 0x00;
    
    PORTB = 0x00;
    PORTD = 0x00;
}

// Initialize timer0 and basic PIC resources
void configPIC_TMRS() {
    INTCONbits.GIEH = 1;
    RCONbits.IPEN = 0;
    INTCONbits.TMR0IF = 0;
    INTCONbits.TMR0IE = 0;
    INTCON2bits.TMR0IP = 1;
    T0CONbits.T08BIT = 0;
    T0CONbits.T0CS = 0;
    T0CONbits.PSA = 0;
    T0CONbits.T0PS2   = 0;
    T0CONbits.T0PS1   = 0;
    T0CONbits.T0PS0   = 1;
    T0CONbits.TMR0ON = 0;
}

void main(void) {
    configPIC(); // Configurar PORTs.
    GLCDinit();
    clearGLCD(0, 7, 0, 127);
    setStartLine(0);
    writeTxt(1, 9, s1);
    writeTxt(2, 8, s2);
    writeTxt(3, 2, s3);
    writeTxt(4, 2, s4);
    __delay_ms(1500);
    clearGLCD(0, 7, 0, 127);
    // Preparar els diferents buffers amb la informació de l'estat.
    sprintf(estatReady, "Ready\n");
    sprintf(estatRunning, "Running...\n");
    sprintf(estatStopped, "Stopped!\n");
    updateGLCD();
    configPIC_TMRS(); // Configurar timers i interrupcions
    // MAIN LOOP
    while (1) {
        if (inputDetector()) { // check falling edge
            if(estatCrono == Ready){
                estatCrono = Running;
		INTCONbits.TMR0IE = 1;
                T0CONbits.TMR0ON = 1;
                TMR0 = 15535;
		clearGLCD(0, 1, 0, 50);
            }
            else if(estatCrono == Running){
                estatCrono = Stopped;
                INTCONbits.TMR0IE = 0;
                T0CONbits.TMR0ON = 0;
                clearGLCD(0, 1, 0, 50);
		updateGLCD();
            }
            else{
                estatCrono = Ready;
                segons = 10;
                decimes = 0;
                clearGLCD(0, 1, 0, 50);
                updateGLCD();
            }
        }
        if(estatCrono == Running){
	    // Només cal actualitzar constantment si el compte enrere està activat
            updateGLCD();// show things on the GLCD smartly!
        }
    }
}