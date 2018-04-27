#include <msp430.h> 

void Shine(void);

unsigned char p2flags = 0, linha = 0, col = 0, ligado = 0,pisca = 0,valor = 0;
void Portas(void)
{
    P1DIR = 0xFF;
    P1OUT = 0;

    //P2.0-3 SAÍDA
    //P2.4-7 ENTRADA PULLUP RESSITOR E INTERRUP
    P2DIR = 0x0F;
    P2IFG = 0;
    P2OUT = 0xF0;
    P2REN = 0xF0;
    P2IE  = 0xF0;
    P2IES = 0xF0;
    P2SEL &= ~(BIT6 + BIT7);
    P2SEL2 &= ~(BIT6 + BIT7);

}

void Clock(void)
{
    DCOCTL = CALDCO_1MHZ;
    BCSCTL1 = CALBC1_1MHZ + XT2OFF;
    BCSCTL2 = DIVS0 + DIVS1;
    BCSCTL3 = XCAP1 + XCAP0;

    __enable_interrupt();

}

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
    Portas();
    Clock();
    ligado = 0;

    do
    {
        if(pisca)
        {
            int i;
            for(i=0; i<10000;i++);
            if(!ligado) 
                  ligado++; 
            else 
                  ligado = 0;
            Shine();
        }

    }while(1);
}

#pragma vector=PORT2_VECTOR
__interrupt void P2_RIT(void)
{
    //debouncer
    p2flags = P2IFG;
    P2IFG = 0;
    P2IE = 0;
    WDTCTL = WDTPW + WDTCNTCL + WDTTMSEL + WDTIS1;//4ms
    IE1 |= WDTIE;
}


#pragma vector=WDT_VECTOR
__interrupt void WDT_RTI(void)
{
    WDTCTL = WDTPW + WDTHOLD;
    char i, bits;
    switch(p2flags & BIT4 + BIT5 + BIT6 + BIT7)
    {
        case BIT4:
            linha = 1;
            bits = BIT4;
            break;
        case BIT5:
            linha = 2;
            bits = BIT5;
            break;
        case BIT6:
            linha = 4;
            bits = BIT6;
            break;
        case BIT7:
            linha = 3;
            bits = BIT7;
            break;
    }

    P2OUT |= BIT0;
    for(i = 1; i <= 4; i++)
    {
        if(P2IN & bits)
            col = i;
        P2OUT = P2OUT<<1;
    }
    if(linha == 1)
    {
        if(col == 1) 
              valor = 1;
        else if(col == 2) 
              valor = 2;
        else if(col == 3) 
              valor = 3;
        else if(col == 4) 
              valor = 10;
    }
    else if(linha == 2)
    {
        if(col == 1) 
              valor = 4;
        else if(col == 2) 
              valor = 5;
        else if(col == 3) 
              valor = 6;
        else if(col == 4) 
              valor = 11;
    }
    else if(linha == 3)
    {
        if(col == 1) 
              valor = 7;
        else if(col == 2) 
              valor = 8;
        else if(col == 3) 
              valor = 9;
        else if(col == 4) 
              valor = 12;
    }
    else if(linha == 4)
    {
        if(col == 1)
        {
              if(!ligado) 
                    ligado++; 
              else 
                    ligado = 0;
      }
        else if(col == 2) 
              valor = 0;
        else if(col == 3)
        {
              if(!pisca) 
                    pisca++; 
              else 
                    pisca = 0;
      }
        else if(col == 4) 
              valor = 13;
    }


    Shine();
    P2OUT = BIT4 + BIT5 + BIT6 + BIT7;
    P2IE = BIT4 + BIT5 + BIT6 + BIT7;
    P2IFG = 0;
    IE1 &= ~WDTIE;
}

void Shine(void)
{
    if(!ligado)
    {
        P1OUT = 0;
    }
    else if(ligado)
    {
        switch(valor)
        {
            case 0:
                P1OUT = 0x77;
                break;
            case 1:
                P1OUT = 0x06;
                break;
            case 2:
                P1OUT = 0xB3;
                break;
            case 3:
                P1OUT = 0x97;
                break;
            case 4:
                P1OUT = 0xC6;
                break;
            case 5:
                P1OUT = 0xD5;
                break;
            case 6:
                P1OUT = 0xF5;
                break;
            case 7:
                P1OUT = 0x07;
                break;
            case 8:
                P1OUT = 0xF7;
                break;
            case 9:
                P1OUT = 0xD7;
                break;
            case 10:
                P1OUT = 0xE7;
                break;
            case 11:
                P1OUT = 0xF4;
                break;
            case 12:
                P1OUT = 0x71;
                break;
            case 13:
                P1OUT =  0xB6;
                break;
        }
    }
}

