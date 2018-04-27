/*
 * Exemplo de configura��o do BCS usando frequencia calibrada
 */

#include <msp430.h>

void config_ini(void);

void main(void) {

    config_ini();
    do{
        // Codigo
    }while(1);
}

void config_ini(void){

    WDTCTL = WDTPW + WDTHOLD;   // Para o WDT

    // Configuracoes do BCS
    // LFXT1 -> Cristal 32.768 Hz
    // DCO -> ~ 16 MHz  (calibra��o)
    // ACLK = LFXT1 = 32.768 Hz
    // MCLK = DCOCLK ~ 16 MHz
    // SMCLK = DCOCLK / 8 ~ 2 MHz

    DCOCTL = CALDCO_16MHZ;    // Freq. Calibrada

    BCSCTL1 = CALBC1_16MHZ;

    BCSCTL2 = DIVS0 + DIVS1;

    BCSCTL3 = XCAP0 + XCAP1; // ou BCSCTL3 = 0x0D;

    while(BCSCTL3 & LFXT1OF);  // prossegue a execucao se o sinal de LFXT1 estiver estavel.

    __enable_interrupt(); // Habilita a geracao de interrupcao

}

