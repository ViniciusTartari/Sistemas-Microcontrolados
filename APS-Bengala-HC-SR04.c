/**
 *			 	 msp430g2553
 *				 ___________
 *				|	    |
 *			    S1T |P1.0	P2.0|S1E
 *			    S2T |P1.1	P2.1|S2E
 *			    S3T |P1.2	P2.2|S3E
 *			    S4T |P1.3	P2.3|S4E
 *				|P1.4	P2.4|
 *				|P1.5	P2.5|
 *				|P1.6	P2.6|
 *				|P1.7	P2.7|
 *				|___________|
 *
 *	SxT = Sensor trigger
 *	SxE = Sensor echo
 * */

#include <msp430.h>

#define SENSOR_0 BIT0
#define SENSOR_1 BIT1

short sensor_id = 0;
float sensors_distances[2];
unsigned int rising_edge = 0;
unsigned int falling_edge = 0;

short sensors_pinouts[2] = {SENSOR_0, SENSOR_1};

float sensor_echo_pulse_diff = 0;
float sensor_distance_cm = 0;


void init_ports()
{
      int i;
      
      P1DIR = BIT2 + BIT4 + BIT5 + BIT6 + BIT7;
      
      //Setup port A for triggering pulses and port B for retrieving the replies from the sensors (via isr).
      for (i = 0; i < 2; i++) {
            P1DIR |= sensors_pinouts[i];
            P2DIR &= ~sensors_pinouts[i];
            P2IE |= sensors_pinouts[i];
            P2IFG &= ~sensors_pinouts[i];
      }
      P1OUT = 0;
      
      P1REN = BIT3; // Resistor do BIT3 habilitado
      
      P1OUT = BIT3; // Resistor de pull-up para BIT3, os demais como saida em
      // nivel logico baixo
      
      P1IES = BIT3; // Interrupcao por borda de descida
      
      P1IFG = 0; // limpa as flags da P1, evitando que uma interrupcao ocorra
      // de forma indevida
      
      P1IE = BIT3;  // Interrupcao do BIT3 da P1 habilitada
      
      P2DIR = 0xFF;  // P2: Todos os bits como saida em nivel baixo
      P2OUT = 0x00;
      
      P2SEL |= BIT2; // saida de sinal PWM do TA1.1
      P2SEL2 &= ~BIT2;0
}

void init_timers()
{
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
      //----------------------------      
      TACTL = TASSEL_1 | MC_1;
      TACCR0 = 20; //1.30ms pulse
      TACCTL1 = OUTMOD_3 | CCIE; //Reset / Set (see page 364 of the user guide)
      TA1CTL = TASSEL_2 | MC_2;
}


#pragma vector=TIMER0_A1_VECTOR
__interrupt void generate_trigger_pulse_isr(void)
{
      switch (__even_in_range(TAIV,0x02)) {
            case TA0IV_TACCR1:
                  P1OUT ^= sensors_pinouts[sensor_id];
                  TACCTL1 &= ~CCIFG;
                  break;
      }
}

#pragma vector=PORT2_VECTOR
__interrupt void echo_reply_isr(void)
{
      const unsigned short sensor = sensors_pinouts[sensor_id]; //Obtain current sensor
      
      if (P2IFG & sensor) { //Has an interrupt occurred?
            if (P2IN & sensor) { //High transition
                  rising_edge = TA1R;
            } else {	//Low transistion
                  falling_edge = TA1R;
                  __bic_SR_register_on_exit(LPM0_bits + GIE); //Wake up CPU (resume main execution)
            }
            P2IES ^= sensor;  //This enables the ISR to be called on both low and high transitions
            P2IFG &= ~sensor; //Clear the pin that generated the interrupt
      }
}


void main(void) {

	init_timers();
	init_ports();

    while (1) {
    	_BIS_SR(LPM0_bits + GIE);	//Turn off CPU w/interrupts
		sensor_echo_pulse_diff = falling_edge - rising_edge;
		sensor_distance_cm = sensor_echo_pulse_diff / 58;
		sensors_distances[sensor_id] = sensor_distance_cm;

		if (sensor_distance_cm <= 10) {
			P1OUT |= BIT6;
		} else {
			P1OUT &= ~BIT6;
		}

		if (sensor_id == 2 - 1) {
			__no_operation();
		}

		//Move to the next sensor
    	sensor_id = ++sensor_id == 2 ? 0 : sensor_id;

    	//Restart timer
    	TACTL = TACLR;
    	TA1CTL = TACLR;
    	//We need to wait for previous sensors to settle before moving to the next sensor
    	__delay_cycles(100000);
    	init_timers();
    }
}

