/* ****************************************************************************
    EXEMPLO Interface USCI - MODO SPI - DAC121S101
        - Geracao de sinal analogico tipo Rampa usando um DAC

    2017-01
******************************************************************************/


#include <msp430.h> 

#define     SYNC    BIT0

void ini_P1_P2(void);
void ini_uCOn(void);
void ini_USCI_B0_SPI(void);

unsigned int valor_dac = 0;
unsigned char estado = 0;

unsigned int temp = 0;

void main(void) {

    ini_P1_P2();
    ini_uCOn();
    ini_USCI_B0_SPI();

    /* Iniciar a TX enviando o Byte mais significativo (MSB)
        BIT             15  14  13  12  11  10  9  8  7  6  5  4  3  2  1  0
        valor_dac >> 8   H   H   H   H   H   H  H  H  L  L  L  L  L  L  L  L
        Resultado ->     0   0   0   0   0   0  0  0  H  H  H  H  H  H  H  H

        UCB0TXBUF   <-- entra apenas LSB <--          H  H  H  H  H  H  H  H

    */
    P1OUT &= ~SYNC;
    UCB0TXBUF = valor_dac >> 8;

    do{

        __bis_SR_register(LPM0_bits + GIE);

    }while(1);

}
//------------------------------------------------------

#pragma vector=USCIAB0TX_VECTOR
__interrupt void RTI_USCI_B0_TX(void){

    IFG2 &= ~UCB0TXIFG;

    while(UCB0STAT & UCBUSY);

    switch(estado){
        case 0:
            UCB0TXBUF = valor_dac; // Envia LSB
            estado = 1;
        break;

        case 1:
            P1OUT |= SYNC;  // Finaliza envio

            if(valor_dac >= 4095){
                valor_dac = 0;
            }else{
                valor_dac++;
            }

            P1OUT &= ~SYNC; // Inicia nova transmissao
            temp =
            UCB0TXBUF = valor_dac >> 8;  // envia MSB
            estado = 0;
        break;
    }
}


void ini_USCI_B0_SPI(void){

    /*
        Modo: SPI
        Clock: SMCLK
        8 bits de dados
    */

    UCB0CTL1 |= UCSWRST; // Deixa a interface inativa ou em estado de RESET

    UCB0CTL0 = UCMSB + UCMST + UCSYNC;

    UCB0CTL1 = UCSSEL1 + UCSWRST;

    UCB0BR1 = 0;
    UCB0BR0 = 1; // Fator de divisao 1

    P1SEL |= BIT5 + BIT7; // Altera funcao dos pinos para UCB0CLK e UCB0SIMO
    P1SEL2 |= BIT5 + BIT7;

    UCB0CTL1 &= ~UCSWRST;

    IFG2 &= ~UCB0TXIFG;

    IE2 |= UCB0TXIE;

}



void ini_uCOn(void){
    WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer

    /* CONFIG. DO BCS
     *
     * DCOCLK ~ 16 MHz (dados de calibracao)
     * VLOCLK = Nao Utilizado
     * LFXT1CLK = 32768 Hz
     *
     * MCLK = DCOCLK ~ 16 MHz
     * SMCLK = DCOCLK/8 ~ 2 MHz
     * ACLK = LFXT1CLK = 32768 Hz
     */

    DCOCTL = CALDCO_16MHZ;
    BCSCTL1 = CALBC1_16MHZ;
    BCSCTL2 = DIVS1 + DIVS0;
    BCSCTL3 = XCAP0 + XCAP1;

     while(BCSCTL3 & LFXT1OF);

     __enable_interrupt();

}



void ini_P1_P2(void){

/* PORTA 1
 *
 *  P1.0 - /SS ou /SYNC
 *  P1.5 - UCB0CLK
 *  P1.7 - UCB0MOSI
 *  P1.X = N.C. - saidas em nivel baixo
 *
 *
 */

    P1DIR = 0xFF;
    P1OUT = SYNC;


/* PORTA 2
 *
 * P2.X - N.C. - saida em nivel baixo
 * Pinos 18 e 19 - Xin e Xout - funcoes permanecem
 *
 */

    P2DIR = 0xFF;

    P2OUT = 0;

}
