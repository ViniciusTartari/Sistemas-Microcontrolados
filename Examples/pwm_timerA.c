/* DISCIPLINA: SISTEMAS MICROCONTROLADOS
 * Prof. Fabio L. Bertotti.
 *
 * TITULO: Implementacao de gerador de sinal PWM
 * usando o TA0 com largura de pulso ajustavel
 * atraves de S2 com debounce.
 *
 * Data: 21/01/2014
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
           /* Sai deste loop para atender as RTIs
            * da Porta 1 e WDT.
            */
    }while(1);
}

void config_ini(void){
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

    // Configuracoes do BCS
    // MCLK = DCOCLK ~ 16 MHz
    // ACLK = LFXT1CLK = 32768 Hz
    // SMCLK = DCOCLK / 8 ~ 2 MHz

    DCOCTL = CALDCO_16MHZ;  // Usando valores de calibracao para 16MHz
    BCSCTL1 = CALBC1_16MHZ;
    BCSCTL2 = DIVS0 + DIVS1;  // Fator divisao = 8 para SMCLK
    BCSCTL3 = XCAP0 + XCAP1;  // Capacitor do cristal ~12.5 pF

    while(BCSCTL3 & LFXT1OF); // Sai deste loop quando clock de LFXT1 estiver estavel

    __enable_interrupt();  // seta o bit GIE - permite geracao de interrupcoes
}

void ini_P1_P2(void){

    // BIT3 como entrada e os demais como saida
    P1DIR = BIT0 + BIT1 + BIT2 + BIT4 + BIT5 + BIT6 + BIT7;

    P1SEL |= BIT6; // saida de sinal PWM do TA0.1
    P1SEL2 &= ~BIT6;

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

    // Fonte de clock: ACLK = 32768 Hz
    // Fator Div.: 1
    // Modo UP sem geracao de int.
    TA0CTL = TASSEL0 + MC0;//clock auxiliar, modo up

    // Modo Reset/set
    // Saida inicializada em nivel alto
    TA0CCTL1 = OUTMOD0 + OUTMOD1 + OUTMOD2 + OUT;

    TA0CCR0 = 327;  // Para periodo de 10 ms

    TA0CCR1 = 0;    // razao ciclica de 0 % - led apagado
}

// RT1 da PORTA 1
#pragma  vector=PORT1_VECTOR
__interrupt void P1_RTI(void){

    P1IFG &= ~BIT3; // Limpa flag para nao gerar interrupcoes
                    // consecutivas

    P1IE &= ~BIT3;  // Desabilita int. do BIT3 da P1

    // WDT configurado como temporizador
    // Clock = SMCLK ~ 2 MHz
    // Fator: 8192
    // Interrupcao a cada 4 ms aprox.
    WDTCTL = WDTPW + WDTTMSEL + WDTCNTCL + WDTIS0;
}

// RTI do WDT
#pragma  vector=WDT_VECTOR
__interrupt void WDT_RTI(void){

    WDTCTL = WDTPW + WDTHOLD;   // Para o WDT

    if( (~P1IN) & BIT3 ) {

        // Alterar a largura do pulso

        if(TA0CCR1 == 0){
            TA0CCR1 = 81;
        }else{

            if(TA0CCR1 >= 327){
                TA0CCR1 = 0;
            }else{
                if(TA0CCR1 == 245){
                    TA0CCR1 += 83;
                }else{
                    TA0CCR1 += 82;
                }
            }
        }

    }

    P1IE = BIT3;  // Interrupcao do BIT3 da P1 habilitada
}
