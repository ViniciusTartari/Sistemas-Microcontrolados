#include "msp430g2231.h"


#define SLV1_EN   BIT0

void ini_ucon(void);    // Inicializa CPU, Sistema de Clock, Watchdog Timer+ e Interrupcoes
void ini_spi(void);     // Inicializa SPI
void tx_data(char data);// Envia caracter pela SPI - Modo Mestre


void main(void){
  char x = 0;
  unsigned long int i = 0;

   ini_ucon();
   ini_spi();

   while(1){

      tx_data(x);          // envia um valor variando de 0 a 255
      if(x >= 255) x = 0;
      else x++;            // ap�s enviar um dado incrementa seu valor

      for(i = 0; i < 200000; i++);  // Atraso

   }   // Loop-infinito

}


void ini_spi(void){

  ///////////////////////////////////////////////////////////
  // Configuracao do pino SLV1_EN - Slave 1 Enable
  // P1.0 - /CS
  P1DIR |= SLV1_EN;
  P1OUT |= SLV1_EN; // Saida em nivel alto
  //                   (escravo desabilitado!)
  ///////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////
  // USICTL0
  //   - USIPE6: habilita pino SDO
  //   - USIPE5: habilita pino SCLK
  //   - USIMST: Modo Mestre
  //   - USIOE: habilita saida de dados SDO
  USICTL0 = USIPE6 + USIPE5 + USIMST + USIOE;
  ///////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////
  // USICTL1
  //   - USICKPH: O dado eh capturado na primeira borda de
  //              SCLK, sendo alterado na proxima borda
  //   - USIIE: Habilita geracao de interrupcao ao termino
  //            de uma transmissao. Util para desabilitar
  //            o escravo.
  // * Depende do dispositivo
  USICTL1 =  USICKPH + USIIE;
  ///////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////
  // USICKCTL
  //   - USISSEL0: seleciona ACLK (32,768 kHz) como fonte
  //               de clock
  //
  USICKCTL =  USISSEL0;
  ///////////////////////////////////////////////////////////
}

void tx_data(char data){
  P1OUT &= ~SLV1_EN;  // Habilita escravo
  USISRL = data;
  USICNT = 0x08;         // Envia dado (8 bits)
}


void ini_ucon(void){
  // Inicializa CPU, Sistema de Clock, Watchdog Timer+ e Interrupcoes
  WDTCTL = WDTPW + WDTHOLD;  // Para WDT+
  // Configuracoes do sistema de clock
  // MCLK = 15 MHz, SMCLK = ACLK = LFXT1 = 32.768 Hz (cristal)
  DCOCTL |= DCO0 + DCO1;
  BCSCTL1 |= XT2OFF + RSEL0 + RSEL1 + RSEL2 + RSEL3;
  BCSCTL2 |= SELS;
  BCSCTL3 |=  XCAP0 + XCAP1 + LFXT1OF;
  while(BCSCTL3 & LFXT1OF);     // Sai do loop quando LFXT1 estiver estavel

  __enable_interrupt();         // Habilita interrupcoes
}


// RTI da USI - neste exemplo usada para desabilitar escravo
#pragma vector=USI_VECTOR
__interrupt void RTI_USI_SPI(void)
{
   USICTL1 &= ~USIIFG; // Limpa flag de interrupcao. Eh limpa
                       // automaticamente quando ocorre uma
                       // escrita em USICNT. Nao eh este o caso,
                       // pois uma nova TX nao eh feita na RTI
   P1OUT |= SLV1_EN;
}


