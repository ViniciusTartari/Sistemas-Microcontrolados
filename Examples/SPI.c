/* CONFIGURACAO USCI para modo SPI a 3 fios sem uso de interrupcao para TX
 *   APLICACAO: Geracao de rampa analogica usando o DAC12S101
 *              Opcao para conectar outros dispositivos de 16b ( 3 Potenciometros Digitais e 1 DDS)
 *              Opcao para conectar outros dispositivos de 32b
 *
 *  uCon-----------
 *             P2.0|- EN DEV 1   - SYNC\  DAC12
 *             P2.1|- EN DEV 2   - SYNC\  POTDG_1
 *             P2.2|- EN DEV 3   - SYNC\  POTDG_2
 *             P2.3|- EN DEV 4   - SYNC\  POTDG_3
 *             P2.4|- EN DEV 5   - SYNC\  DDS
 *             P1.5|- UCB0CLK    - CLK
 *             P1.6|- UCB0SOMI   - Nao usado
 *             P1.7|- UCB0SIMO   - SDI do DEV
 *
 * Prof. Dr. Fabio L. Bertotti
 * DATA: 22/04/2013
 */



#include <msp430g2553.h>

#define SYNC_DAC12_1    BIT0
#define SYNC_POTDG_1    BIT1
#define SYNC_POTDG_2    BIT2
#define SYNC_POTDG_3    BIT3
#define SYNC_DDS___1    BIT4
//#define SYNC_DEV32_1    BITX
//#define SYNC_DEV32_2    BITY

#define DAC12_1      1
#define POTDG_1      2
#define POTDG_2      3
#define POTDG_3      4
#define DDS___1      5

#define DEV32_1      1
#define DEV32_2      2


#define DAC12_1_EN    P2OUT &= ~SYNC_DAC12_1;
#define DAC12_1_DS    P2OUT |=  SYNC_DAC12_1;

#define POTDG_1_EN    P2OUT &= ~SYNC_POTDG_1;
#define POTDG_1_DS    P2OUT |=  SYNC_POTDG_1;

#define POTDG_2_EN    P2OUT &= ~SYNC_POTDG_2;
#define POTDG_2_DS    P2OUT |=  SYNC_POTDG_2;

#define POTDG_3_EN    P2OUT &= ~SYNC_POTDG_3;
#define POTDG_3_DS    P2OUT |=  SYNC_POTDG_3;

#define DDS___1_EN    P2OUT &= ~SYNC_DDS___1;
#define DDS___1_DS    P2OUT |=  SYNC_DDS___1;

// #define DEV32_1_EN     P2OUT &= ~SYNC_DEV32_1;
// #define DEV32_1_DS     P2OUT |=  SYNC_DEV32_1;

// #define DEV32_2_EN     P2OUT &= ~SYNC_DEV32_2;
// #define DEV32_2_DS     P2OUT |=  SYNC_DEV32_2;



void config_ini(void);
void ini_portas(void);
void ini_USCI_B0_SPI(void);
void tx_word_16b_spi(unsigned int word_tx_spi, unsigned char device);
void tx_word_32b_spi(unsigned long int word_tx_spi, unsigned char device_32b);



unsigned int  valor_dac = 0;

int main(void) {

   config_ini();
   ini_portas();
   ini_USCI_B0_SPI();


   do{

        tx_word_16b_spi(valor_dac, DAC12_1);   // Geracao de rampa analogica na saida do DAC12
        valor_dac++;
        if(valor_dac > 4095) valor_dac = 0;

        tx_word_16b_spi(0x1122, POTDG_1);    // Envio de valores para os outros dispositivos
        tx_word_16b_spi(0x4488, POTDG_2);
        tx_word_16b_spi(0xAABB, POTDG_3);
        tx_word_16b_spi(0xCCDD, DDS___1);

        // Envio para dispositivos DEV32_1 e DEV32_2 nao implementado

   }while(1);

}


void tx_word_32b_spi(unsigned long int word_tx_spi, unsigned char device_32b){

    volatile unsigned char Byte_0, Byte_1, Byte_2, Byte_3;


    switch(device_32b){         // Seleciona dispositivo a ser habilitado
        case 1:
            // DEV32_1_EN;
        break;
        case 2:
            // DEV32_2_EN;
        break;
        default:
            // DEV32_1_EN;
    }

    Byte_0 = (unsigned char) word_tx_spi;   // Separa os Bytes da palavra para enviar os MSBits primeiro
    word_tx_spi = word_tx_spi >> 8;
    Byte_1 = (unsigned char) word_tx_spi;
    word_tx_spi = word_tx_spi >> 8;
    Byte_2 = (unsigned char) word_tx_spi;
    word_tx_spi = word_tx_spi >> 8;
    Byte_3 = (unsigned char) word_tx_spi;

    UCB0TXBUF = Byte_3;
    while(UCB0STAT & UCBUSY);               // sai do while quando Shift Register esta vazio
    UCB0TXBUF = Byte_2;
    while(UCB0STAT & UCBUSY);               // sai do while quando Shift Register esta vazio
    UCB0TXBUF = Byte_1;
    while(UCB0STAT & UCBUSY);               // sai do while quando Shift Register esta vazio
    UCB0TXBUF = Byte_0;
    while(UCB0STAT & UCBUSY);               // sai do while quando Shift Register esta vazio

    switch(device_32b){         // Seleciona dispositivo a ser desabilitado
        case 1:
            // DEV32_1_DS;
        break;
        case 2:
            // DEV32_2_DS;
        break;
        default:
            // DEV32_1_DS;
    }

}



void tx_word_16b_spi(unsigned int word_tx_spi, unsigned char device){

    switch(device){         // Seleciona dispositivo a ser habilitado
        case 1:
            DAC12_1_EN;
        break;
        case 2:
            POTDG_1_EN;
        break;
        case 3:
            POTDG_2_EN;
        break;
        case 4:
            POTDG_3_EN;
        break;
        case 5:
            DDS___1_EN;
        break;
        default:
            DAC12_1_EN;
    }

    UCB0TXBUF = word_tx_spi >> 8;           // Envia 8 bits mais significativos
    while(UCB0STAT & UCBUSY);               // sai do while quando Shift Register esta vazio
    UCB0TXBUF = (unsigned char) word_tx_spi; // Envia os 8 bits menos significativos
    while(UCB0STAT & UCBUSY);               // sai do while quando Shift Register esta vazio

    switch(device){         // Seleciona dispositivo a ser desabilitado
        case 1:
            DAC12_1_DS;
        break;
        case 2:
            POTDG_1_DS;
        break;
        case 3:
            POTDG_2_DS;
        break;
        case 4:
            POTDG_3_DS;
        break;
        case 5:
            DDS___1_DS;
        break;
        default:
            DAC12_1_DS;
    }
}


void config_ini(void){
    // MCLK = DCO = 16MHz
    // ACLK = LFXT1 = 32.768 Hz
    // SMCLK = DCO/2 = 8 MHz

    WDTCTL = WDTPW + WDTHOLD;   // Para o WDT

    DCOCTL = CALDCO_16MHZ;   // Valor para DCOCTL fornecido pelo fabricante para 16MHz
    BCSCTL1 = CALBC1_16MHZ;  // Valor para BCSCTL1 fornecido pelo fabricante para 16MHz
    BCSCTL2 = DIVS0;
    BCSCTL3 = XCAP0 + XCAP1;

    while(BCSCTL3 & LFXT1OF); // sai do loop quando oscilador LFXT1 estiver estavel

    __enable_interrupt();  // Habilita geracao de interrupcoes
}


void ini_portas(void){

// Inicializacao da Porta 1

    P1REN = BIT0 + BIT1 + BIT2 + BIT3 + BIT4;  // Como pinos config. como entrada, todos os resistores config. para pull-down.
    P1OUT = 0;

// Inicializacao da Porta 2

    P2DIR |= SYNC_DAC12_1 + SYNC_POTDG_1 + SYNC_POTDG_2 + SYNC_POTDG_3 + SYNC_DDS___1;

    P2REN = 0;

    P2OUT |= SYNC_DAC12_1 + SYNC_POTDG_1 + SYNC_POTDG_2 + SYNC_POTDG_3 + SYNC_DDS___1;

}


void ini_USCI_B0_SPI(void){

     // OBS.: Esta ordem de configuracao deve ser seguida para a SPI funcionar corretamente.
     //       * Ordem sugerida pelo User guide (16.3.1 USCI Initialization and Reset)

     UCB0CTL1 |= UCSWRST; // Desabilita USCI_B0

     UCB0CTL0 = UCMSB + UCMST + UCSYNC;
        // UCCKPH e UCCKPL = 0 para borda descida coincidir com o meio do bit de dados
        // UCMSB: Primeiro bit deve ser o MSB
        // UC7BIT = 0 para transmissao de 8 bits
        // UCMODEx = 00 para SPI no modo 3 pinos
        // UCSYNC = 1 para selecionar modo sincrono

     UCB0CTL1 = UCSSEL1 + UCSWRST;
        // UCSSELx = 10 para selecionar clock SMCLK
        // UCSWRST = 1 para manter USCI desabilitada durante as configuracoes

     UCB0BR0 = 4;   // Freq. de SCLK = SMCLK/4 = 8 MHz / 4 = 2 MHz.
     UCB0BR1 = 0;

     // P1.5 -> UCB0CLK  (Linha de clock do mestre - saida)
     // P1.6 -> UCB0SOMI (Linha de dados de saida do mestre - entrada do escravo)
     // P1.7 -> UCB0SIMO (Linha de dados de entrda do mestre - saida do escravo)
     P1SEL |= BIT5 + BIT6 + BIT7; // Habilita porta para periferico
     P1SEL2 |= BIT5 + BIT6 + BIT7; // Habilita porta para periferico

     UCB0CTL1 &= ~UCSWRST;  // USCI habilitada
}


