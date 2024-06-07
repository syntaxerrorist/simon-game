#include <avr/io.h>
#include "timer.h"
#include "spi.h"
#include "buzzer.h"
#include "adc.h"
#include "uart.h"


// DEFINED VALUES
#define STATE_LFSR 0x10893997
#define LFSR_MASK 0xE2023CAB
#define SEGS_EF  0b00111110
#define SEGS_BC  0b01101011
#define SEGS_OFF 0b01111111
#define SEGS_SUCC 0b00000000
#define SEGS_FAIL 0b01110111

// ** Global Variables **

volatile uint16_t sequence_length = 1; // sequence length tracker; uint16 to store sequences up to 65535
volatile uint8_t current_step = 0; // tracks current step
volatile uint8_t segs [] = {SEGS_OFF, SEGS_OFF};  //segs initially off
volatile uint8_t pb_debounced = 0xFF;
uint32_t lsfr = STATE_LFSR; // store student number in hex

// Timer variables
extern uint32_t program_timer;
uint32_t start_time;

// Variable holders for segment displays
uint8_t LED_RIGHT;
uint8_t LED_LEFT;


// Generate a random number between 0 and 3 based on student number
void gen_ran_number(void) {
	uint8_t bit = lsfr & 1;
	lsfr >>= 1;
	if (bit) {
		lsfr ^= LFSR_MASK;
	}
}

// Pushbutton Initialization Function
void pb_init(void) {

    //PBs PA4-7, enable pullup resistors
    PORTA.PIN4CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN5CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN6CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN7CTRL = PORT_PULLUPEN_bm;
}

/*
 * Function for pushbutton debounce mechanism
 * From Tutorial 10.
 * 
 * This function is acknowledged by the TCB timer ISR
*/
void pb_debounce(void) {

    static uint8_t count0 = 0, count1 = 0;

    uint8_t pb_sample = PORTA.IN;
    uint8_t pb_changed = pb_sample ^ pb_debounced;

    count1 = (count1 ^ count0) & pb_changed;
    count0 = ~count0 & pb_changed;

    pb_debounced ^= (count1 & count0); 
}

// States for the state machine
typedef enum {
    GENSEQUENCE,
    PLAY_SEQUENCE,
    WAIT_PLAYBACK,
    USER_INPUT,
    SUCCESS,
    FAIL,
    SCORE,
    RESET
}
states;

// This function is used to increment the current step and update the start time to the current program timer
void increment_step(void){
    start_time = program_timer;
    current_step++;
}

// Main Program
int main(void) {

    // Initialize all functions
    pb_init();
    tcb0_init();
    tcb1_init();
    spi_init();
    buzzer_init();
    adc_init();
    uart_init();
    gen_ran_number();


    // Initialize state (state machine)
    states state = GENSEQUENCE;

    // Declare variables with default values
    uint8_t pb_c = 0xFF; // pb current
    uint8_t pb_p = 0xFF; // pb previous
    uint8_t pb_rising = 0x00; // rising edge
    uint8_t pb_falling = 0x00; // falling edge

    uint16_t score = 0;
    uint8_t disp_score_left = 0;
    uint8_t disp_score_right = 0;
    uint16_t playback_length = 0;
    uint32_t new_state;
    uint32_t prev_state = lsfr;
    

    while (1) {

        // Detect rising and falling edges of the pushbuttons and store the 
        // results in variables pb_falling and pb_rising 
        pb_p = pb_c;
        pb_c = pb_debounced;

        uint8_t pb_changed = pb_p ^ pb_c;
        pb_rising = pb_changed & pb_c;
        pb_falling = pb_changed & pb_p;

        // ADC playback delay (from Tutorial 12)
        uint32_t playback_delay = 250UL + ((1750UL * ADC0.RESULT) >> 8);
        uint32_t play_time = playback_delay >> 1;

// ------------------------------------------------------------------------------------ 
        switch (state) {
        case GENSEQUENCE:
            state = PLAY_SEQUENCE;
            
            // Calculate lsfr & 0b11
            uint8_t lsfr_bits = lsfr & 0b11;
            
            uint16_t tone_per;

            switch (lsfr_bits) {
                case 0b00:
                    tone_per = TONE1_PER;
                    segs[0] = SEGS_EF;
                    segs[1] = SEGS_OFF;
                    break;
                case 0b01:
                    tone_per = TONE2_PER;
                    segs[0] = SEGS_BC;
                    segs[1] = SEGS_OFF;
                    break;
                case 0b10:
                    tone_per = TONE3_PER;
                    segs[0] = SEGS_OFF;
                    segs[1] = SEGS_EF;
                    break;
                case 0b11:
                    tone_per = TONE4_PER;
                    segs[0] = SEGS_OFF;
                    segs[1] = SEGS_BC;
                    break;
                default:
                    break;
            }
            
            TCA0.SINGLE.PERBUF = tone_per;
            TCA0.SINGLE.CMP0BUF = tone_per >> 1;
            
            // Turn buzzer on
            TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;
            
            start_time = program_timer;
            playback_length++;
            break;

        default:
            break;            
// ------------------------------------------------------------------------------------         
        case PLAY_SEQUENCE: 
        if ((program_timer - start_time) >= play_time) {
            TCA0.SINGLE.CTRLA &= ~TCA_SINGLE_ENABLE_bm; // buzzer off
            segs[0] = SEGS_OFF;
            segs[1] = SEGS_OFF;
            
            if ((program_timer - start_time) >= playback_delay) {
                gen_ran_number();

                /* Short-hand way of writing an if statement
                   if condition is true, assign state to GENSEQUENCE, else USER_INPUT */
                state = (playback_length < sequence_length) ? GENSEQUENCE : USER_INPUT;

                // If playback length is less than sequence length
                if (state == GENSEQUENCE) {
                    new_state = lsfr;

                // If playback length is greater than sequence length
                } else if (current_step == 0) {
                    lsfr = prev_state;
                }
                
            }
        }
        break;
// ------------------------------------------------------------------------------------ 
        case WAIT_PLAYBACK: 
        if ((pb_rising & (PIN4_bm | PIN5_bm | PIN6_bm | PIN7_bm)) || 
            (program_timer - start_time) >= play_time) {
                
            // Turn buzzer off
            TCA0.SINGLE.CTRLA &= ~TCA_SINGLE_ENABLE_bm;;
            segs[0] = SEGS_OFF;
            segs[1] = SEGS_OFF;
            
            if ((program_timer - start_time) >= playback_delay) {
                gen_ran_number();
                state = USER_INPUT;
            }
        }
        break;

// ------------------------------------------------------------------------------------ 
        case USER_INPUT: 
            // Check if current step equals the sequence length
            if (current_step == sequence_length) {
                // Set state to SUCCESS
                state = SUCCESS;
                // Set segment values for display
                segs[0] = SEGS_SUCC;
                segs[1] = SEGS_SUCC;
                // Set start time
                start_time = program_timer;
                break;
            }
            
            // Check which button was pressed using bitwise AND
            switch (pb_falling & (PIN4_bm | PIN5_bm | PIN6_bm | PIN7_bm)) {
                // S1 Pressed
                case PIN4_bm:
                    // Check LSFR value for correct sequence
                    if ((lsfr & 0b11) == 0b00) {
                        state = PLAY_SEQUENCE;
                    } else {
                        state = FAIL;
                    }

                    // Set segment values for display
                    segs[0] = SEGS_EF;
                    segs[1] = SEGS_OFF;

                    // Set tone and enable the buzzer
                    TCA0.SINGLE.PERBUF = TONE1_PER;
                    TCA0.SINGLE.CMP0BUF = TONE1_PER >> 1;
                    TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;

                    increment_step(); // increment the current step and updates the start time to program timer
                    break;

                // S2 Pressed
                case PIN5_bm:
                    // Check LSFR value for correct sequence
                    if ((lsfr & 0b11) == 0b01) {
                        state = PLAY_SEQUENCE;
                    } else {
                        state = FAIL;
                    }

                    // Set segment values for display
                    segs[0] = SEGS_BC;
                    segs[1] = SEGS_OFF;

                    // Set tone and enable the buzzer
                    TCA0.SINGLE.PERBUF = TONE2_PER;
                    TCA0.SINGLE.CMP0BUF = TONE2_PER >> 1;
                    TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;

                    increment_step(); // increment the current step and updates the start time to program timer
                    break;

                // S3 Pressed
                case PIN6_bm: 
                    // Check LSFR value for correct sequence
                    if ((lsfr & 0b11) == 0b10) {
                        state = PLAY_SEQUENCE;
                    } else {
                        state = FAIL;
                    }

                    // Set segment values for display
                    segs[0] = SEGS_OFF;
                    segs[1] = SEGS_EF;

                    // Set tone and enable the buzzer
                    TCA0.SINGLE.PERBUF = TONE3_PER;
                    TCA0.SINGLE.CMP0BUF = TONE3_PER >> 1;
                    TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;

                    increment_step(); // increment the current step and updates the start time to program timer
                    break;

                // S4 Pressed
                case PIN7_bm:
                    // Check LSFR value for correct sequence
                    if ((lsfr & 0b11) == 0b11) {
                        state = PLAY_SEQUENCE;
                    } else {
                        state = FAIL;
                    }

                    // Set segment values for display
                    segs[0] = SEGS_OFF;
                    segs[1] = SEGS_BC;

                    // Set tone and enable the buzzer
                    TCA0.SINGLE.PERBUF = TONE4_PER;
                    TCA0.SINGLE.CMP0BUF = TONE4_PER >> 1;
                    TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;

                    increment_step(); // increment the current step and updates the start time to program timer
                    break;
                default:
                    break;
            }
            break;

// ------------------------------------------------------------------------------------ 
        case SUCCESS:
            // Check if the play time has elapsed
            if ((program_timer - start_time) >= play_time) {
                segs[0] = SEGS_OFF;
                segs[1] = SEGS_OFF;
                // Check if the playback delay has elapsed, if so, transition to SCORE state
                if ((program_timer - start_time) >= playback_delay) {
                    state = SCORE;

                    // Reset LSFR value
                    lsfr = prev_state;

                    // Increase sequence length, score, and display score
                    sequence_length++;
                    score++;
                    disp_score_left = (disp_score_left + 1) % 10; // modulo operator to wrap display score values within range [0,9]

                    // Increment disp_score_right if disp_score_left reaches 0
                    if (disp_score_left == 0) {
                        disp_score_right = (disp_score_right + 1) % 10; // modulo operator to wrap display score values within range [0,9]
                    }

                    /* Update LED display
                     * If the right score digit is greater than 0, turn on the right LED segment,
                     * otherwise turn it off */
                    display_led(disp_score_left, disp_score_right);
                    segs[1] = LED_LEFT;
                    segs[0] = (disp_score_right > 0) ? LED_RIGHT : SEGS_OFF;

                    // Reset playback length, current step and start timne
                    playback_length = 0;
                    current_step = 0;
                    start_time = program_timer;
                }
            }
        break;
// ------------------------------------------------------------------------------------ 
        case FAIL:
            // Check if the playtime has elapsed
            if ((program_timer - start_time) >= play_time) {

                // Turn buzzer off
                TCA0.SINGLE.CTRLA &= ~TCA_SINGLE_ENABLE_bm;

                // Set both segments to display "FAIL"
                segs[0] = segs[1] = SEGS_FAIL;

                // If elapsed time exceeds playback delay, transition to RESET state and reset start time
                if ((program_timer - start_time) >= playback_delay) {

                    state = RESET;                    
                    start_time = program_timer;
                }
            }
        break;
// ------------------------------------------------------------------------------------ 
        case SCORE:
            // Check if play time has elapsed, if so, turn off display segments
            if ((program_timer - start_time) >= play_time){
                segs[0] = segs[1] = SEGS_OFF;

                // Check if playback delay has elapsed, if so transition to GENSEQUENCE state
                if ((program_timer - start_time) >= playback_delay){
                    state = GENSEQUENCE;
                }
                break;
            }
        break;
// ------------------------------------------------------------------------------------ 
        case RESET:
            // Check if the time since start exceeds the play time and the playback delay
            if ((program_timer - start_time) >= play_time && (program_timer - start_time) >= playback_delay) {

                // Turn off display segments and transition to GENSEQUENCE state
                segs[0] = segs[1] = SEGS_OFF;
                state = GENSEQUENCE;

                // Generate a new LSFR state and random number
                lsfr = new_state;
                gen_ran_number();

                // Set the previous state, sequence length, and step variables
                prev_state = lsfr;
                sequence_length = 1;
                current_step = 0;

                // Reset the playback length and score display variables
                playback_length = 0;
                disp_score_left = disp_score_right = 0;
            }
        break;
        }
    }
}
    
