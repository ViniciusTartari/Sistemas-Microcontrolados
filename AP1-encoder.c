#include <msp430.h>

//encoder fio vermelhop1.1 fio marron p1.2 fio preto GND

#define SEGA BIT0
#define SEGB BIT1
#define SEGC BIT2
#define SEGD BIT3
#define SEGE BIT4
#define SEGF BIT5
#define SEGG BIT6

#define ZERO    SEGA + SEGB + SEGC + SEGD + SEGE + SEGF
#define UM      SEGB + SEGC
#define DOIS    SEGA + SEGB + SEGD + SEGG + SEGE;
#define TRES    SEGA + SEGB + SEGC + SEGD + SEGG
#define QUATRO  SEGB + SEGC + SEGF + SEGG
#define CINCO   SEGA + SEGF + SEGG + SEGC + SEGD
#define SEIS    SEGA + SEGF + SEGE + SEGG + SEGD + SEGC
#define SETE    SEGA + SEGB + SEGC
#define OITO    SEGA + SEGB + SEGC + SEGD + SEGE + SEGF + SEGG
#define NOVE    SEGA + SEGB + SEGC + SEGD + SEGF + SEGG

void setaBCD(void);
void Portas_config(void);
void uConfig(void);

char valor = 0, coiso = 0, p1flags = 0, p1inflags = 0;

void main(void) {
    WDTCTL = WDTPW + WDTHOLD;
    uConfig();
    Portas_config();
    setaBCD();

    do
    {

    } while(1);

}

#pragma vector=PORT1_VECTOR
__interrupt void P1_RTI(void)
{
    p1flags = P1IFG;// pega se tem evento de interrupçao ou nao
    P1IE &= ~(BIT1 + BIT2);//
    P1IFG &= ~(BIT1 + BIT2);
    WDTCTL = WDTPW + WDTTMSEL + WDTCNTCL + WDTIS1;
    IE1 |= WDTIE;
}

#pragma vector=WDT_VECTOR
__interrupt void WDT_RTI(void) {

    WDTCTL = WDTPW + WDTHOLD;
    if(!coiso)
    {
        switch(p1flags & (BIT1 + BIT2))
        {
            case BIT1:
                if(~P1IN & BIT1)
                {
                    if(valor < 9) 
                          valor++;
                    coiso++;
                }
                break;

            case BIT2:
                if(~P1IN & BIT2)
                {
                    if(valor > 0) 
                          valor--;
                    coiso++;
                }
                break;
        }
    }
    else
    {
        coiso = 0;
    }
    setaBCD();

    P1IFG &= ~(BIT1 + BIT2);
    P1IE |= BIT1 + BIT2;

}

void Portas_config(void) {
    //configurações de portas
    //PORTAS P1 RECEBEM ENCODER P1.1 E P1.2
    P1DIR = ~(BIT1 + BIT2);
    P1REN = BIT1 + BIT2;
    P1OUT = BIT1 + BIT2;//AS PORTAS DE ENTRADA FICAM COM RESISTOR DE PULL UP

    P1IFG = 0;
    P1IE = BIT1 + BIT2;
    P1IES = BIT1 + BIT2;

    //portas p2 todas em saída E EM BAIXO
    P2DIR = 0xFF;
    P2OUT = 0;
    P2SEL &= ~(BIT6);
}

void uConfig(void) {
    //configurações de clock
    DCOCTL = CALDCO_1MHZ;
    BCSCTL1 = CALBC1_1MHZ + XT2OFF;
    BCSCTL2 = DIVS1 + DIVS0;
    BCSCTL3 = XCAP0 + XCAP1;

   //while(BCSCTL3 & LFXT1OF);

    __enable_interrupt();
}

void setaBCD() {
    switch(valor){
        case 0:
            P2OUT = ZERO;
            break;
        case 1:
            P2OUT = UM;
            break;
        case 2:
            P2OUT = DOIS;
            break;
        case 3:
            P2OUT = TRES;
            break;
        case 4:
            P2OUT = QUATRO;
            break;
        case 5:
            P2OUT = CINCO;
            break;
        case 6:
            P2OUT = SEIS;
            break;
        case 7:
            P2OUT = SETE;
            break;
        case 8:
            P2OUT = OITO;
            break;
        case 9:
            P2OUT = NOVE;
            break;
    }
}

