#include <msp430.h>
#include <stdlib.h>
#include <stdio.h>

unsigned int  capturaPrev=0, capturaNow=0, tempoCaptura=0;
unsigned char stable=0, bit=0, tempInt=0, tempDec=0, humInt=0, humDec=0, checksum=0, segundos=0, level=0, flagbit=0, recepcao=0, contingencia=0, new=0;
float Temp=0, Hum=0, tempMax=0, tempMin=0, humMax=0, humMin=0;

void Config_Clock()
{
      WDTCTL = WDTPW | WDTHOLD;
      DCOCTL = CALDCO_16MHZ;
      BCSCTL1 = CALBC1_16MHZ;
      BCSCTL2 = DIVS0;
      BCSCTL3 = XCAP1 + XCAP0;
      while(BCSCTL3 & LFXT1OF);
}

void Config_Portas()
{
      P2DIR = 0xFF; 
      P2OUT = BIT0; 
}

void Config_Timer()
{
      TA0CTL = TASSEL0;             //ACLK
      TA1CTL = TASSEL1 + TACLR;     //SMCLK
      TA0CCTL0 = CCIE;              //Capture/Compare interrupt enable
      TA0CCR0 = 32767;              //Geracao de interrupcao a cada 1 segundo
      __enable_interrupt();
}

void Inicia_Leitura()
{
      TA1CCTL1 &= ~CCIFG;           //Capture/Compare interrupt flag
      TA1CCTL1 = CCIE;              //Capture/Compare interrupt enable
      TA1CCR1 = 39999;              //Testar******(59999)
      TA1CTL = TASSEL1 + ID1 + TACLR + MC1; //SMCLK / 4 ~ 2MHz - continuo
      P2OUT &= ~BIT0;               //Inverte para comecar comunicacao
      new = 0;
}

//RTI para monitorar o tempo entre leituras
#pragma vector=TIMER0_A0_VECTOR
__interrupt void RTI_tempo_leitura()
{
      segundos++;
      if(stable==1 && segundos>=30)
      {
            segundos=0;
            new=1;
      }
      else if (stable==0 || contingencia==1)
      { //Entra aqui na primeira leitura ou numa releitura em caso de erro
            stable=1;
            segundos=0;
            contingencia=0;
            new=1;
      }
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void RTI_leitura()
{
      capturaNow = TA1CCR0;
      TA1CCTL0 &= ~CCIFG;
      if(level>1) //>1 - Compara
      {
            if(capturaNow < capturaPrev)
                  tempoCaptura = (65535 - capturaPrev) + capturaNow;
            else
                  tempoCaptura = capturaNow - capturaPrev;
      }
      capturaPrev = capturaNow;
      if(level>=1 && level<4)
            level++;
      else if(level == 4 && tempoCaptura >= 70 && tempoCaptura <= 120 && flagbit == 0)
            flagbit=1;
      else if(flagbit==1 && bit < 40 )
      {
            bit++;
            if(tempoCaptura >= 5 && tempoCaptura <=80)
                  recepcao = 0;
            else if(tempoCaptura >= 80 && tempoCaptura <=160)
                  recepcao = 1;
            if(bit<=8)
                  humInt += recepcao << (8 - bit);
            else if(bit<=16)
                  humDec += recepcao << (16 - bit);
            else if(bit<=24)
                  tempInt += recepcao << (24 - bit);
            else if(bit<=32)
                  tempDec += recepcao << (32 - bit);
            else if(bit<=40)
                  checksum += recepcao << (40 - bit);
            flagbit=0;
      }
      
      if (bit==40)
      {
            TA1CTL = 0; 
            TA1CCTL0 &= ~CCIE; //para a interrupcao
            P2OUT |= BIT0; 
            P2DIR |= BIT0; 
            P2SEL &= ~BIT0; 
            if((unsigned char)(humInt + humDec + tempInt + tempDec) == checksum)
            {
                  Temp= tempInt + (float)(tempDec/100);
                  Hum = humInt + (float)(humDec/100);
            }else
                  contingencia=1;
      }
}

#pragma vector=TIMER1_A1_VECTOR
__interrupt void RTI_comunicacao()
{
      TA1CCTL1 &= ~CCIFG;
      TA1CTL &= ~TAIFG;
      bit=0;
      level=1;
      humInt=0;
      humDec=0;
      tempInt=0;
      tempDec=0;
      checksum=0;
      TA1CCTL1 &= ~(CCIFG + CCIE);
      P2OUT |= BIT0;
      TA1CCTL0 = CM1 + CM0 + CAP + CCIE;
      P2DIR &= ~BIT0;
      P2SEL |= BIT0;
      P2SEL2 &= ~BIT0;
}

void main() 
{
      Config_Clock();
      Config_Portas();
      Config_Timer();
      TA0CTL |= MC0; //Model control
      while(1)
            if(new != 0)
                  Inicia_Leitura();
}

