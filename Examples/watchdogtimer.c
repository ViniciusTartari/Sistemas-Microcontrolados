#include <msp430.h> 

void ini_P1_P2(void);
void ini_uCon(void);

void main(void) {

    long i = 0;
    ini_P1_P2();
    ini_uCon();

    /* CONFIG. DO WDT
     *
     * Modo: watchdog
     * Clock: ACLK = 4096 Hz
     * Saida do contador:
     *
     *  SAIDA cont          Q15         Q13           Q8        Q6
     *  Contagens          32768        8192          512       64
     *
     *  ACLK (4096 Hz)       8s          2s          125ms      15,625ms
     *  SMCLK  (2 MHz)     16,384ms     4,096ms      256us      32us
     *  ACLK   (3 kHz)      10,92s   >> 2,73s <<         ....
     *
     */
    WDTCTL = WDTPW + WDTCNTCL + WDTSSEL + WDTIS0;
    do{
        for(i=0;i<1000000;i++){ // delay ~ 1s
        }
        if( (~P1IN) & BIT3 ){ // Entra se S2 for pressionada
                        for(;;); // Loop infinito
        }
        P1OUT ^= BIT0;
        WDTCTL = WDTPW + WDTCNTCL + WDTSSEL + WDTIS0; // Limpa contador do WDT

    }while(1);
}

void ini_uCon(void){

    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

    /* CONFIG. DO BCS
     *
     * DCOCLK = 16MHz (calibrada)
     * LFXT1  = 32.768 Hz
     *
     * MCLK = DCOCLK = 16 MHz
     * SMCLK = DCOCLK / 8 = 2 MHz
     * ACLK = LFXT1 / 8 = 4.096 Hz
     *
     */

    DCOCTL = CALDCO_16MHZ;
//    BCSCTL1 = CALBC1_16MHZ + DIVA0 + DIVA1;
    BCSCTL1 = CALBC1_16MHZ + DIVA1;
    BCSCTL2 = DIVS0 + DIVS1;
//    BCSCTL3 = XCAP0 + XCAP1;
    BCSCTL3 = LFXT1S1;

    /* Teste do bit LFXT1OF para verificar se LFXT1 esta estavel
     *
     *                         ----> LFXT1OF
     *                        |
     * BIT      7 6 5 4 3 2 1 0
     * BCSCTL3  X X X X X X X Y
     * LFXT1OF  0 0 0 0 0 0 0 1 AND bit-a-bit
     *         -----------------
     *          0 0 0 0 0 0 0 Y
     *
     *          Y -> 1, oscilador LFXT1 NAO estabilizou na freq. do cristal
     *          Y -> 0, oscilador LFXT1 ESTABILIZOU na freq. do cristal
     *
     * while( BCSCTL3 & LFXT1OF){
     *    // Nao sai do Loop ate oscilador estabilizar
     * }
     *
     */
//    while( BCSCTL3 & LFXT1OF );

    // Setar o bit GIE de R2 da CPU para que ela aceite req. de int.
    __enable_interrupt();
}

void ini_P1_P2(void){

    /* CONFIG. DA PORTA 1
     * P1.0 - LED VM - saida em nivel baixo
     * P1.3 - S2 - Entrada com resistor de pull-up
     * P1.x - saida em nivel baixo.
     *
     * bit          7 6 5 4 3 2 1 0
     * P1DIR  ini   0 0 0 0 0 0 0 0
     * BIT3         0 0 0 0 1 0 0 0
     * ~BIT3        1 1 1 1 0 1 1 1
     *
     * bit          7 6 5 4 3 2 1 0
     * P1DIR  ini   1 1 1 1 0 1 1 1 <= ~BIT3(1 1 1 1 0 1 1 1)
     */
    P1DIR = ~BIT3;
    P1REN = BIT3;
    P1OUT = BIT3;

    // P1IE   -> Int. nao utilizada
    // P1IES  ->  Int. nao utilizada
    // P1IFG  ->  Int. nao utilizada
    // P1SEL  ->  Pino ini. conectado a porta -> nao precisa config.
    // P1SEL2 ->  Pino ini. conectado a porta -> nao precisa config.
    // P1IN   ->  Nao configuravel -> somente de leitura


    /* CONFIG. DA PORTA 2
     *
     * P2.x - Todos como saida em nivel baixo
     *
     * P2.6 (pino 19) - Conectado no XT1 e no BCS
     * P2.7 (pino 18) - Conectado no XT1 e no BCS
     */
    P2DIR = 0xFF;
    P2OUT = 0;
}
