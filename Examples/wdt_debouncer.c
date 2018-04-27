#include <msp430.h> 

void config_ini(void);   // Prototipos das funcoes
void ini_P1_P2(void);

void main(void) {

    config_ini();
    ini_P1_P2();

    IE1 |= WDTIE;  // Habilita a geracao de interrupcao pelo WDT

    do{
           /* Sai deste loop para atender as RTIs
            * da Porta 1 e WDT.
            *
            */

    }while(1);
}


void config_ini(void){
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

    // Configuracoes do BCS
    // MCLK = DCOCLK ~ 15 MHz
    // ACLK = LFXT1CLK = 32768 Hz
    // SMCLK = DCOCLK / 8 ~ 1,875 MHz

    DCOCTL = DCO0 + DCO1;  // DCOx = 3
    BCSCTL1 = XT2OFF + RSEL0 + RSEL1 + RSEL2 + RSEL3; // RSELx = 15
    BCSCTL2 = DIVS0 + DIVS1;  // Fator divisao = 8 para SMCLK
    BCSCTL3 = XCAP0 + XCAP1;  // Capacitor do cristal ~12.5 pF

    while(BCSCTL3 & LFXT1OF); // Sai deste loop quando clock de LFXT1 estiver estavel

    __enable_interrupt();  // seta o bit GIE - permite geracao de interrupcoes
}


void ini_P1_P2(void){

    // BIT3 como entrada e os demais como saida
    P1DIR = BIT0 + BIT1 + BIT2 + BIT4 + BIT5 + BIT6 + BIT7 ;

    P1REN = BIT3; // Resistor do BIT3 habilitado

    P1OUT = BIT3; // Resistor de pull-up para BIT3, os demais como saida em
                  // nivel logico baixo

    P1IES = BIT3; // Interrupcao por borda de descida

    P1IFG = 0; // limpa as flags da P1, evitando que uma interrupcao ocorra
               // de forma indevida

    P1IE = BIT3;  // Interrupcao do BIT3 da P1 habilitada

    P2DIR = 0xFF;  // P2: Todos os bits como saida em nivel baixo
    P2OUT = 0x00;

}


// RT1 da PORTA 1
#pragma  vector=PORT1_VECTOR
__interrupt void P1_RTI(void){

    P1IFG &= ~BIT3; // Limpa flag para nao gerar interrupcoes
                    // consecutivas

    P1IE &= ~BIT3;  // Desabilita int. do BIT3 da P1

    // WDT configurado como temporizador
    // Clock = SMCLK ~ 1,875 MHz
    // Fator: 8192
    // Interrupcao a cada 4,3 ms aprox.
    WDTCTL = WDTPW + WDTTMSEL + WDTCNTCL + WDTIS0;
}


// RTI do WDT
#pragma  vector=WDT_VECTOR
__interrupt void WDT_RTI(void){

    WDTCTL = WDTPW + WDTHOLD;   // Para o WDT

    if( (~P1IN) & BIT3 ) {

        P1OUT ^= BIT0; // Tecla valida! entao alterna
                       // estado do led vermelho

    }

    P1IE = BIT3;  // Interrupcao do BIT3 da P1 habilitada
}
