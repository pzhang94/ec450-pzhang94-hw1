/***************************************************************************************
 *  MSP430 Blink the LED Demo -  Toggle P1.0
 *
 *  This version uses the Watchdog Timer ("WDT") in interval mode.
 *  The default system clock is about 1.1MHz.  The WDT is configured to be driven by this
 *  clock and produce an interrupt every 8K ticks ==> interrupt interval = 8K/1.1Mhz ~ 7.5ms
 *  When the WDT interrupt occurs, the WDT interrupt handler is called.
 *  This handler decrements a global counter variable (blink_counter).  If the counter
 *  reaches 0, then the handler toggles the LED and reinitializes blink_counter to the
 *  value of another global variable (blink_interval).
 *  The result is that the LED toggles at intervals of ~7.5ms * blink_interval.
 *  (the program starts with blink_interval=67, but this value can be changed in the debugger
 *
 *  NOTE: Between interrupts the CPU is OFF!
 ***************************************************************************************/

#include <msp430g2553.h>

volatile unsigned int blink_interval;  // number of WDT interrupts per blink of LED
volatile unsigned int blink_counter;   // down counter for interrupt handler
volatile unsigned int state;
int main(void) {
	// setup the watchdog timer as an interval timer
	WDTCTL =(WDTPW + // (bits 15-8) password
				   // bit 7=0 => watchdog timer on
				   // bit 6=0 => NMI on rising edge (not used here)
				   // bit 5=0 => RST/NMI pin does a reset (not used here)
		   WDTTMSEL + // (bit 4) select interval timer mode
		   WDTCNTCL +  // (bit 3) clear watchdog timer counter
				  0 // bit 2=0 => SMCLK is the source
				  +1 // bits 1-0 = 01 => source/8K
		   );
	IE1 |= WDTIE;		// enable the WDT interrupt (in the system interrupt register IE1)

	P1DIR |= 0x01;					// Set P1.0 to output direction

	// initialize the state variables
	blink_interval=67;                // the number of WDT interrupts per toggle of P1.0
									// call this 1 unit
	blink_counter=blink_interval;     // initialize the counter
    
	int state = 0; // keeps track of the state of the sequence
    // our sequence is a 18 step process (0 - 17)
	
    P1OUT = 0; // initializes the light as off
	
    _bis_SR_register(GIE+LPM0_bits);  // enable interrupts and also turn the CPU off!
}

interrupt void WDT_interval_handler()
{
    if(--blink_counter == 0) // decrements the blink_counter
    {
        if(state >= 0 && state <= 5) // the first initial 3 short streams of light
        {
            P1OUT ^= 1;                     // toggle LED on P1.0
            blink_counter = blink_interval; // reset the down counter
            state++;                        // sends the sequence to the next state
        }
        else if(state == 6) // the long period of no light leading up to the 3 long streams of light
        {
            blink_counter = 3 * blink_interval; // increases the interval time (between states) to 3 times the original interval time
            state++;
        }
        else if(state >= 7 && state <= 11 && state % 2 == 1) // the 3 long streams of light
        {
            P1OUT ^= 1;
            blink_counter = 3 * blink_interval;
            state++;
        }
        else if(state >= 7 && state <= 11 && state % 2 == 0) // the short periods of no light between the long streams of light
        {
            P1OUT ^= 1;
            blink_counter = blink_interval;
            state++;
        }
        else if(state == 12) // the long period of no light leading up to the 3 short streams of light
        {
            P1OUT = 0;
            blink_counter = 3 * blink_interval;
            state++;
        }
        else if(state >= 13 && state <= 17) //  the final 3 short streams of light of the sequence
        {
            P1OUT ^= 1;
            blink_counter=blink_interval;
            state++;
        }
        else if(state == 18) // since our sequence is a 18 step process (0 - 17), step 18 is equivalent to step 0
        {
            P1OUT = 0; // initializes the light as off
            state = 0; // resets the state of our sequence
            blink_counter = 7 * blink_interval; // sets the time between sequences to be 7 times the original interval time
        }
    }
}



// DECLARE function WDT_interval_handler as handler for interrupt 10
// using a macro defined in the msp430g2553.h include file
ISR_VECTOR(WDT_interval_handler, ".int10")
