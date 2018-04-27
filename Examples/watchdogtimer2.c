#include <msp430.h> 

void ini_P1_P2(void);
void ini_uCon(void);

unsigned char SEGUNDO = 0, MINUTO = 0, HORA = 0;

void main(void) {

    ini_P1_P2();
    ini_uCon();


    /* CONFIG. DO WDT
     * Clock ACLK = 32768 Hz
     * Modo: Temporizador
     * Saida: Q15 -> 32768 contagens
     * Interrupcao: habilitada
     * tempo de interrupcao: 1 segundo
     */
    WDTCTL = WDTPW + WDTTMSEL + WDTSSEL + WDTCNTCL;
    IE1 |= WDTIE; //SETANDO O BIT0

    do{

    }while(1);
}
//--------------------------------------------------

//RTI DO WDT
#pragma vector=WDT_VECTOR
__interrupt void WDT_RTI(void){
    /*a cpu entra nesta rti a cada 1 segundo
     *É necessario limpar a flag de interrupção?
     *R: não pois é limpa automaticamente pelo hardware.
     * precisa limpar qnd tem mais de um evento que pode causar interrupçao
     */

    P1OUT ^= BIT0;//bit0 = ledvermelho

    if(SEGUNDO >= 59){
        SEGUNDO = 0;
        if(MINUTO>=59){
            MINUTO=0;
            if(HORA>=23){
                HORA=0;
            }
            else{
                HORA++;
            }
        }
        else{
            MINUTO++;
        }
    }
    else{
        SEGUNDO++;
    }
}


void ini_uCon(void){

    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

    /* CONFIG. DO BCS
     *
     * MCLK = DCOCLK ~ 1 MHz
     * SMCLK = DCOCLK/8 ~ 125 KHz
     * ACLK = LFXT1CLK ~ 32768 Hz
     */

    DCOCTL = CALDCO_1MHZ;
    BCSCTL1 = CALBC1_1MHZ;
    BCSCTL2 = DIVS0 + DIVS1;
    BCSCTL3 = XCAP0 + XCAP1;

    /*
     *bcsctl3
     *                               -->LFXT1OF
     *                              |
     * bits           7 6 5 4 3 2 1 0
     * bcsctl3        x x x x x x x y
     * BIT0ouLFXT1OF  0 0 0 0 0 0 0 1
     * AND bab        ----------------
     *                0 0 0 0 0 0 0 y
     * se y=1, entao LFXT1 n está sintonizado ainda com o cristal
     *
     * se y==0, entao LFXT1 tem a freq nominal do cristal, ou seja, ja pode ser utilizado.
     *
     * O programa só pode funcionar com o sinal de LFXT1 sintonizado na freq. do cristal, caso contrário
     * o relógio nao vai funcionar corretamente
     */

    while(BCSCTL3 & LFXT1OF){//utiliza constante p alterar bit de registrador(testando o bit lfxt1of do registrador bcsctl3)

        __enable_interrupt(); //seta bit GIE de R2, ou seja, habilita a cpu a aceitar requisicoes de interrupçao.
    }
}

void ini_P1_P2(void){

    /* CONFIG. DA PORTA 1
     *
     * P1.0 - LED VM - indica os segundos - nivel baixo saida
     * P1.x - saida em nivel baixo. -> n conectados
     */
    P1DIR = 0xFF;

    P1OUT = 0;

    /* CONFIG. DA PORTA 2
     *
     * P2.x - Todos como saida em nivel baixo
     *
     * P2.6 (pino 19) - Conectado no LFXT1 e no BCS
     * P2.7 (pino 18) - Conectado no LFXT1 e no BCS
     */
    P2DIR = 0xFF;
    P2OUT = 0;//sem o cristal
}
