#pragma once

// #define MATRIX_ROWS 10
// #define MATRIX_COLS 15

#define DEBOUNCE 30

#define TAPPING_TOGGLE 2  // 2回タップでロックするように変更

// PS/2 TrackPoint
#define PS2_CLOCK_PIN D0
#define PS2_DATA_PIN  D1
#define PS2_INT_INIT()  do {                        \
    EICRA |= ((1<<ISC01));                          \
} while(0)
#define PS2_INT_ON()  do {                          \
    EIMSK |= ((1<<INT0));                           \
} while(0)
#define PS2_INT_OFF() do {                          \
    EIMSK &= ~((1<<INT0));                          \
} while(0)
#define PS2_INT_VECT   INT0_vect
#define PS2_MOUSE_SCROLL_BTN_MASK (1<<PS2_MOUSE_BTN_MIDDLE)
#define PS2_MOUSE_SCROLL_DIVISOR_H 2
#define PS2_MOUSE_SCROLL_DIVISOR_V 2
#define PS2_MOUSE_X_MULTIPLIER 1
#define PS2_MOUSE_Y_MULTIPLIER 1

#define PS2_MOUSE_INIT_DELAY 1000
// #define PS2_MOUSE_SCROLL_MASK 0x07

// Bitbang I2C (MCP23017)
#define I2C_SCL D2
#define I2C_SDA D3
#define MCP_ADDR 0x20
