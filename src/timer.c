#include "timer.h"
#include "spi.h"

extern uint8_t digit1, digit2;
extern uint8_t segs [];
extern uint8_t pattern1, pattern2;
volatile uint32_t program_timer = 0;

void pb_debounce(void);

// From tutorial 12
void tcb0_init(void) {
    cli();
//    TCB0.CTRLA = TCB_CLKSEL_DIV2_gc;    // Configure CLK_PER/2
    TCB0.CTRLB = TCB_CNTMODE_INT_gc;    // Configure TCB0 in periodic interrupt mode
    TCB0.CCMP = 33333;                   // Set interval for 10ms (33333 clocks @ 3.3 MHz)
    TCB0.INTCTRL = TCB_CAPT_bm;         // CAPT interrupt enable
    TCB0.CTRLA = TCB_ENABLE_bm;         // Enable
    sei(); 
}
void tcb1_init(void){
    cli();
    TCB1.CTRLB = TCB_CNTMODE_INT_gc;    // Configure TCB0 in periodic interrupt mode
    TCB1.CCMP = 3333;                   // Set interval for 1ms (33333 clocks @ 3.3 MHz)
    TCB1.INTCTRL = TCB_CAPT_bm;         // CAPT interrupt enable
    TCB1.CTRLA = TCB_ENABLE_bm;         // Enable
    sei(); 
}

// From tutorial 12
ISR(TCB0_INT_vect) {
    pb_debounce();
    static uint8_t digit = 0;    

    if (digit) {
        spi_write(segs[0] | (0x01 << 7));
    } else {
        spi_write(segs[1]);
    }
    digit = !digit;

    TCB0.INTFLAGS = TCB_CAPT_bm;
}

// Use TCB1 to increment program timer
ISR(TCB1_INT_vect){
    program_timer++;
    TCB1.INTFLAGS = TCB_CAPT_bm;
}
