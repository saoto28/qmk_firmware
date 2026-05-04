#include "quantum.h"
#include <avr/io.h>

void keyboard_pre_init_kb(void) {
    setPinOutput(B5);
    setPinOutput(B6);
    keyboard_pre_init_user();
}

void matrix_init_kb(void) {
    TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM10);
    TCCR1B = (1 << WGM12) | (1 << CS11); 

    matrix_init_user();
}