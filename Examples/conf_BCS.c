/*
 * Exemplo de configuração do BCS com frequencia aproximada obtida
 *   a partir da tabela no datasheet.
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
    // DCO -> ~ 15 MHz
    // ACLK = LFXT1 = 32.768 Hz
    // MCLK = DCOCLK ~ 15MHz
    // SMCLK = DCOCLK / 8 ~ 1,875 MHz

    DCOCTL = DCO0 + DCO1;   // ou   DCOCTL = 0x60;

    BCSCTL1 = XT2OFF + RSEL0 + RSEL1 + RSEL2 + RSEL3; // ou BCSCTL1 =  0x8F;

    BCSCTL2 = DIVS0 + DIVS1;

    BCSCTL3 = XCAP0 + XCAP1; // ou BCSCTL3 = 0x0D;

    while(BCSCTL3 & LFXT1OF);  // prossegue a execucao se o sinal de LFXT1 estiver estavel.

    __enable_interrupt(); // Habilita a geracao de interrupcao

}
