#include <avr/io.h>
#include <avr/interrupt.h>


// Defined frequency values based on my student number for TCA0.SINGLE.PER
#define TONE1_PER 8952 
#define TONE2_PER 10639
#define TONE3_PER 6700
#define TONE4_PER 17903

// Enum to store LED seg values
enum LED_Num{
    LED_0 = 0b00001000,
    LED_1 = 0b01101011,
    LED_2 = 0b01000100,
    LED_3 = 0b01000001,
    LED_4 = 0b00100011,
    LED_5 = 0b00010001,
    LED_6 = 0b00010000,
    LED_7 = 0b01001011,
    LED_8 = 0b00000000,
    LED_9 = 0b00000001,
};


void display_led(uint8_t left, uint8_t right);
void buzzer_init(void);
