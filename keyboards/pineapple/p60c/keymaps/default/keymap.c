#include QMK_KEYBOARD_H
#include "ps2.h"

enum layer_names {
    _BASE,
    _FNC,
    _NUM
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
     /* L00      L01      L02      L03      L04      L05      L06        R00      R01      R02      R03      R04      R05      R06  */
     /* L10      L11      L12      L13      L14      L15                 R10      R11      R12      R13      R14      R15      R16      R17  */
     /* L20      L21      L22      L23      L24      L25      L26        R20      R21      R22      R23      R24      R25               R27  */
     /* L30      L31      L32      L33      L34      L35                 R30      R31      R32      R33      R34               R36      R37  */
     /* L40      L41      L42      L43      L44               L46        R40      R41      R42      R43      R44               R46      R47  */
    [_BASE] = LAYOUT(
        KC_ESC,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,      KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSPC,
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,               KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_BSLS,
        KC_LSFT, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_GRV,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,          KC_ENT,
        KC_LCTL, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,               KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH,          KC_UP,   KC_RSFT,
        TT(_FNC),KC_CAPS, KC_LGUI, KC_LALT, TT(_NUM),         KC_SPC,    KC_SPC,  TT(_FNC),KC_RALT, KC_RCTL, KC_LEFT,          KC_DOWN, KC_RGHT
    ),
    [_FNC] = LAYOUT(
        _______, KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,     KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  KC_PDOT,
        KC_ESC,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,              KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  _______,
        _______, _______, KC_PSCR, _______, _______, KC_HOME, _______,   KC_PGUP, KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, _______,          _______,
        _______, KC_MUTE, KC_VOLD, KC_VOLU, _______, KC_END,             KC_PGDN, KC_BRID, KC_BRIU, KC_WHOM, BL_DOWN,          KC_HOME, _______,
        _______, _______, _______, _______, _______,          _______,   _______, _______, _______, _______, KC_PGDN,          KC_END,  KC_PGDN
    ),
    [_NUM] = LAYOUT(
        _______, _______, _______, _______, _______, _______,  _______,  _______, _______, _______, _______,  _______, _______, _______,
        _______, S(KC_1), S(KC_2), S(KC_3), S(KC_4), S(KC_5),            S(KC_6), S(KC_7), S(KC_8), S(KC_9),  S(KC_0), S(KC_EQL),_______,_______,
        _______, KC_1,    KC_2,    KC_3,    KC_4,    KC_5,     _______,  KC_6,    KC_7,    KC_8,    KC_9,     KC_0,    KC_EQL,          _______,
        _______, KC_PPLS, KC_PMNS, KC_PAST, KC_PSLS, KC_EQL,             KC_LBRC, KC_RBRC, S(KC_LBRC),S(KC_RBRC),KC_GRV,       _______, _______,
        _______, _______, _______, _______, _______,           _______,  _______, _______, _______, _______,  _______,         _______, _______
    )
};

void ps2_mouse_init_user(void) {
    ps2_host_send(0xF3);  // set sample rate
    ps2_host_send(200);   // 200 samples/sec
}