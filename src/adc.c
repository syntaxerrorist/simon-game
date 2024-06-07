#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>

void adc_init(){

   /*
    * From Tutorial 8:
    *
    * ADC Initialization Function:
    * - Enables the ADC (Analog-to-Digital Converter)
    * - Sets a clock prescaler of /2
    * - Configures the ADC timebase for 4 CLK_PER cycles @ 3.3 MHz (1us)
    * - Selects VDD (supply voltage) as the reference for ADC conversion
    * - Sets the sample duration to 64
    * - Configures ADC for free-running mode and left adjustment of the result
    * - Selects AIN2 (potentiometer R1) as the input for ADC conversion
    * - Sets the ADC to use 8-bit resolution and start the conversion immediately
    */
   
    ADC0.CTRLA = ADC_ENABLE_bm;
    ADC0.CTRLB = ADC_PRESC_DIV2_gc;
    ADC0.CTRLC = (4 << ADC_TIMEBASE_gp) | ADC_REFSEL_VDD_gc;
    ADC0.CTRLE = 64;
    ADC0.CTRLF = ADC_FREERUN_bm | ADC_LEFTADJ_bm;
    ADC0.MUXPOS = ADC_MUXPOS_AIN2_gc;
    ADC0_COMMAND = ADC_MODE_SINGLE_8BIT_gc | ADC_START_IMMEDIATE_gc;

}
