/* DISCIPLINA: SISTEMAS MICROCONTROLADOS
 * Prof. Fabio L. Bertotti.
 *
 * TITULO: Exemplo 3 TA - Modo Captura
 *         Medicao da Freq. do DCO usando como referencia
 *         o sinal ACLK.
 *         O pino 2 do uCon deve ser conectado no pino 3 atraves de jumper
 *
 *  >>> Usando o modo de economia de energia 0 (LPM0)
 *
 * Data: 17/12/2013
 */

#include <msp430.h> 

unsigned char i = 0;

unsigned int val_atual = 0, val_anterior = 0, contagens = 0;

unsigned long int FREQ_DCO = 0;


void config_ini(void);   // Prototipos das funcoes
void ini_P1_P2(void);
void ini_TA0(void);


void main(void) {

    config_ini();
    ini_P1_P2();
    ini_TA0();



    do{
         /* Neste loop nada de "util" eh feito. A CPU pode
          * ser desligados para economizar energia.
          */

           _BIS_SR(LPM0_bits + GIE); // Entra no LPM0

    }while(1);

}


void config_ini(void){
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

    // Configuracoes do BCS
    // MCLK = DCOCLK ~ 15 MHz
    // ACLK = LFXT1CLK / 8 = 4096 Hz
    // SMCLK = DCOCLK ~ 15 MHz

    DCOCTL = DCO0 + DCO1;  // DCOx = 3

    BCSCTL1 = XT2OFF + RSEL0 + RSEL1 + RSEL2 + RSEL3 + DIVA0 + DIVA1; // RSELx = 15
    // ou BCSCTL2 = 0;  // Fator divisao = 1 para SMCLK
    // ACLK = LFXT1CLK / 8 = 4096 kHz (para aumentar resolucao)

    BCSCTL3 = XCAP0 + XCAP1;  // Capacitor do cristal ~12.5 pF

/*
    // Configuracao usando dados de calibracao gravados na memoria flash de dados
    // Freq DCO aprox. 16MHz    |    ACLK = LFXT1CLK / 8 = 4096 kHz

    DCOCTL = CALDCO_16MHZ;   // DCO com freq. calibrada de 16 MHz

    BCSCTL1 = CALBC1_16MHZ + DIVA0 + DIVA1;
*/

    while(BCSCTL3 & LFXT1OF); // Sai deste loop quando clock de LFXT1 estiver estavel

    __enable_interrupt();  // seta o bit GIE - permite geracao de interrupcoes
}


void ini_P1_P2(void){

    // BIT1 como entrada para TA0.0
    P1DIR = BIT0 + BIT3 + BIT2 + BIT4 + BIT5 + BIT6 + BIT7 ;

    P1OUT = 0; // Todos os pinos de saida da P1 em nivel logico baixo


    P2DIR = 0xFF;  // P2: Todos os bits como saida em nivel baixo
    P2OUT = 0x00;

    // Pino 2 (P1.0) como saida de ACLK
    // Pino 3 (P1.1) como entrada para TA0.CCI0A
    P1SEL = BIT0 + BIT1;
}

void ini_TA0(void){

    // Sinal de clock = SMCLK = DCOCLK ~ 15 MHz
    // Modo continuo
    TA0CTL = TASSEL1 + MC1;

    TA0CCTL0 = CM0 + CAP + CCIE;

}

// RTI do TIMER A0 - CCR0
#pragma  vector=TIMER0_A0_VECTOR
__interrupt void TimerA0_CC0_RTI(void){

    val_atual = TA0CCR0;

    if(i == 1){

        if(val_atual < val_anterior){

            contagens = (65535 - val_anterior) + val_atual;
        }else{
            contagens = val_atual - val_anterior;
        }

        FREQ_DCO = contagens*4096.0;
    }

    val_anterior = val_atual;
    i = 1;
}
