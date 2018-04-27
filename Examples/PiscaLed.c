#include <msp430.h> 

void main(void) {
    unsigned long i =0;
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	//Configuração da porta 1
    P1DIR = 0xFF;//todos bits 1
    P1OUT=0;
    //Configuraçao da porta 2
    P2DIR = 0xFF;
    P2OUT= 0;
    do{
        //delay
        for(i=0;i<100000;i++){
            //n faz nada
        }
        P1OUT^=BIT0;
    }while(1);
	return 0;
}
