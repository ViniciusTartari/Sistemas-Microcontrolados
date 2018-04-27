/* DISCIPLINA: SISTEMAS MICROCONTROLADOS
 * Prof. Fabio L. Bertotti.
 *
 * TITULO: Exemplo 2 TA - Modo Continuo
 *         A cada 1s e 0,5s alterna os estados dos
 *         LEDs vermelho e verde, respectivamente.
 *         TA modo Continuo, CCRO, CCR1,
 *         ACLK (LFXT1 - 32768 Hz) com interrup. do CCR0 e do CCR1.
 *
 *         INCLUSO: Implementacao Debounce para chave S2
 *                  Ao pressionar S2 o estado do led vermelho muda.
 *
 *  >>> Usando o modo de economia de energia 0 (LPM0)
 */

#include <msp430.h> 

void config_ini(void);   // Prototipos das funcoes
void ini_P1_P2(void);
void ini_TA0(void);

void main(void) {

    config_ini();
    ini_P1_P2();
    ini_TA0();

    IE1 |= WDTIE;  // Habilita a geracao de interrupcao pelo WDT


    do{
         /* Neste loop nada de "util" eh feito. A CPU pode
          * ser desligados para economizar energia.
          *
          * Como o DCO fornece clock para o WDT, eh preciso
          * ter cuidado ao desliga-lo. Entao, o modo de economia
          * de energia mais adequado eh o LPM0, pois
          * somente a CPU eh desligada.
          *
          * Para otimizar mais o consumo de energia, neste exemplo eh
          * possivel associar os modos LPM0 e LPM3 nas RTIs da
          * Porta 1 e WDT, respectivamente. Ao entrar na RTI da
          * Porta 1 o WDT precisa ser iniciado, portanto eh necessario
          * clock do DCO (SMCLK alimenta o WDT), exigindo o LPM0.
          * Quanto entrar na RTI do WDT, ocorre sua parada e o WDT
          * nao funcionara ate que ocorra uma interrupcao de P1.
          * Portanto, o DCO nao sera usado, podendo entrar no LPM3
          * no final da RTI do WDT.
          *
          * Para fazer o teste da condicao acima, eh necessario
          * comentar a linha de codigo abaixo (ref. a LPM0) e
          * descomentar as linhas comentadas no final das RTIs da
          * P1 e do WDT.
          *
          */

           _BIS_SR(LPM0_bits + GIE); // Entra no LPM0

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

void ini_TA0(void){

    // taclk = ACLK = 32768 Hz, Modo Continuo
    // CCR0 com Interrup. a cada 1s
    // CCR1 com Interrup. a cada 0,5s

    TA0CTL = TASSEL0 + MC1;

    TA0CCTL0 = CCIE;//habilita interrupçao do modulo 0

    TA0CCTL1 = CCIE;//habilita interrupçao do modulo 1

    TA0CCR0 = 32767;

    TA0CCR1 = 16383;
}

// RTI do TIMER A0 - CCR0
#pragma  vector=TIMER0_A0_VECTOR
__interrupt void TimerA0_CC0_RTI(void){

    TA0CCR0 += 32767;
    P1OUT ^= BIT0;
}

// RTI do TIMER A0 - CCR1,2
#pragma  vector=TIMER0_A1_VECTOR
__interrupt void TimerA0_CC12_RTI(void){

    switch(TA0IV){//TA0IV -> VETOR DE INTERRUPÇÃO
        case 2:
            TA0CCR1 += 16383;
            P1OUT ^= BIT6;
        break;
        case 4:

        break;
    }
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

    _BIS_SR(LPM0_bits + GIE); // Entra no LPM0
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

    _BIS_SR(LPM3_bits + GIE); // Entra no LPM3
}
