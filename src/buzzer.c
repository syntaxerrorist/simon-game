#include "buzzer.h"

void buzzer_init(void) {

   /* 
    * From Tutorial 12:
    *
    * Buzzer Initialization Function:
    * 
    * - Enables the buzzer by setting PORTB PIN0 as an output
    * - Uses TCA0 to drive the buzzer
    * - Sets the prescaler of /1 -> TCA0 clk - 3.33MHz
    * - By using prescaler of /1, it enables the TCA0 module 
    *   to operate at the same frequency as the input clock (3.33MHz)
    * 
    * - Utilizes a single slope PWM and enables WO0 as the
    *   output signal connected to PB0
    * 
    */

    PORTB.DIRSET = PIN0_bm;
    PORTB.OUTSET = PIN0_bm;
    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1_gc;
    TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_SINGLESLOPE_gc | TCA_SINGLE_CMP0EN_bm;
}