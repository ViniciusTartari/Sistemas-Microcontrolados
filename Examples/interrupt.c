#include <msp430.h> 
//debouncer
void ini_P1_P2(void);
void ini_uCon(void);

unsigned char p1_flags = 0, p1_flags2;

void main(void) {

    ini_P1_P2();
    ini_uCon();


    /* CONFIG. DO WDT
     * CLOCK
     * SAIDA WDT            Q6     Q9       Q13     Q15
     *                      64     512      8192    32768
     * ACLK 32768 Hz        1,9ms  15,6ms   250ms   1s
     *
     * SMCLK 125Khz         512us  >4,1ms<  65,5ms  262ms
     *
     * Clock SMLCK
     * Modo: Temporizador
     * Saida: Q9 -> 512 contagens
     * Interrupcao: habilitada
     * tempo de interrupcao: 4,1ms
     */
    WDTCTL = WDTPW + WDTTMSEL + WDTCNTCL + WDTIS1;
    IE1 |= WDTIE; //SETANDO O BIT0

    do{

    }while(1);
}
//--------------------------------------------------

//RTI P1
#pragma vector = PORT1_VECTOR
__interrupt void P1_RTI(void){
    /*
     * 1 - Precisa limpar flag de interrupcao?
     * R:Sim, pois o hardware nao limpa automaticamente.
     *  Como limpar:
     *       P1IFG &= ~BIT3; // limpa somente flag BIT3
     *
    */


    p1_flags = P1IFG;  // LIMPANDO A leitura das flags
    P1IE &= ~(BIT3 + BIT5); //DESABILITANDO A INTERRUPÇAO DE P1.3 E P1.5
}


//RTI DO WDT
#pragma vector=WDT_VECTOR
__interrupt void WDT_RTI(void){
    /* 1 - Parar o temporizador
     *
     * 2 - Validar a tecla-> realizo a acao da tecla _> alterna o estado do led vermelho
     *
     * 3 - Limpar flag de interrupçao de p1.3
     *
     * 4- Habilitar a geraçao de interrupçao de P1.3
     */
    WDTCTL = WDTPW + WDTHOLD;

    p1_flags2 = P1IFG;
    switch(p1_flags & (BIT3 + BIT5)){
        case BIT3:
            if((~P1IN) & BIT3){// APENAS BIT 3 PASSA SE FOR 1 A CHAVE ESTA ABERTA SE FOR ZERO TA FECHADA
                   P1OUT ^= BIT0;//led vermelho
               }
            p1_flags2 &= ~BIT3; // limpa somente o bit3
            break;

        case BIT5:
            if((P1IN) & BIT3){
                   P1OUT ^= BIT6; //led verde
               }
            p1_flags2 &= ~BIT5;
            break;

        default:
            P1OUT ^= BIT0;//led vermelho
            P1OUT ^= BIT6; //led verde
            break;
    }

    P1IFG = p1_flags2;//limpando as flags
    P1IE |= BIT3 + BIT5; // interrupçao
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
     * P1.3 - s2 - entrada com resistor de pull up
     * P1.5 - s3 - entrada com resistor de pull down
     * P1.x - saida em nivel baixo. -> n conectados
     *
     * -----------P1.3 entrada com resistor de pull up
     *  |
     *   / s2
     *  |
     * _|_
     *  -
     *
     *  ^
     *  |
     *   /
     *  |
     *  --> P1.5 entrada com resistor de pull down
     */

    P1DIR = ~(BIT3 + BIT5);
    P1REN = BIT3 + BIT5;
    P1OUT = BIT3;
    P1IES = BIT3; //detecçao por borda de descida
    P1IFG = 0; //Limpa todas as flags de interrup da porta 1
    P1IE = BIT3 + BIT5; //interrupçao de p1.3 e p1.5 habilitada

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
