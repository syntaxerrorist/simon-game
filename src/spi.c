
#include "spi.h"
#include "buzzer.h"


// SPI Initialization Function. ** From Tutorial 12 **
void spi_init(void) {
    cli();
    PORTMUX.SPIROUTEA = PORTMUX_SPI0_ALT1_gc;   // SPI pins on PC0-3
    PORTC.DIRSET = (PIN0_bm | PIN2_bm);         // Set SCK (PC0) and MOSI (PC2) as outputs
    PORTA.OUTSET = PIN1_bm;                     // DISP LATCH as output, initially high
    PORTA.DIRSET = PIN1_bm;    
    
    SPI0.CTRLA = SPI_MASTER_bm;                 // Master, /4 prescaler, MSB first  
    SPI0.CTRLB = SPI_SSD_bm;                    // Mode 0, client select disable, unbuffered
    SPI0.INTCTRL = SPI_IE_bm;                   // Interrupt enable    
    SPI0.CTRLA |= SPI_ENABLE_bm;                // Enable
    sei();
}

void spi_write(uint8_t b) {
    SPI0.DATA = b;
}

// Extern variables to assign values from LED map
extern uint8_t LED_RIGHT;
extern uint8_t LED_LEFT;


// Display LED map
void display_led(uint8_t left, uint8_t right) {
    const uint8_t LED_MAP[] = {LED_0, LED_1, LED_2, LED_3, LED_4, LED_5, LED_6, LED_7, LED_8, LED_9}; // store constants inside array
    
    // Check if the value of 'right' is within the valid range of 0 to 9
    if (right <= 9) {
        // Assign the LED value corresponding to 'right' from the LED_MAP array to LED_RIGHT
        LED_RIGHT = LED_MAP[right];
    }
    
    // Check if the value of 'left' is within the valid range of 0 to 9
    if (left <= 9) {
        // Assign the LED value corresponding to 'left' from the LED_MAP array to LED_LEFT
        LED_LEFT = LED_MAP[left];
    }
}

// SPI ISR
ISR(SPI0_INT_vect) {
    // Rising edge DISP LATCH
    PORTA.OUTCLR = PIN1_bm; 
    PORTA.OUTSET = PIN1_bm;
    SPI0.INTFLAGS = SPI_IF_bm;
}
