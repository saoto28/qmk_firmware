#include "matrix.h"
#include "wait.h"
#include "print.h"
#include <util/delay.h>
#include "quantum.h"

#define I2C_SCL D2
#define I2C_SDA D3
#define MCP_ADDR 0x20

#define MCP_IODIRA 0x00
#define MCP_IODIRB 0x01
#define MCP_GPPUA  0x0C
#define MCP_OLATB  0x15
#define MCP_GPIOA  0x12

static void i2c_delay(void) { _delay_us(5); }
static void sda_hi(void) { gpio_set_pin_input_high(I2C_SDA); }
static void sda_lo(void) { gpio_set_pin_output(I2C_SDA); gpio_write_pin_low(I2C_SDA); }
static void scl_hi(void) { gpio_set_pin_input_high(I2C_SCL); }
static void scl_lo(void) { gpio_set_pin_output(I2C_SCL); gpio_write_pin_low(I2C_SCL); }

// OCR1A, OCR1B の値を保持する変数
static uint8_t caps_duty = 0;
static uint8_t layer_duty = 0;

static void i2c_start(void) {
    sda_hi(); scl_hi(); i2c_delay();
    sda_lo(); i2c_delay();
    scl_lo(); i2c_delay();
}

static void i2c_stop(void) {
    sda_lo(); scl_hi(); i2c_delay();
    sda_hi(); i2c_delay();
}

static bool i2c_write_byte(uint8_t b) {
    for (int i = 7; i >= 0; i--) {
        if (b & (1 << i)) sda_hi(); else sda_lo();
        i2c_delay(); scl_hi(); i2c_delay(); scl_lo();
    }
    sda_hi(); i2c_delay(); scl_hi(); i2c_delay();
    bool ack = !gpio_read_pin(I2C_SDA);
    scl_lo();
    return ack;
}

static uint8_t i2c_read_byte(bool ack) {
    uint8_t b = 0;
    sda_hi();
    for (int i = 7; i >= 0; i--) {
        scl_hi(); i2c_delay();
        if (gpio_read_pin(I2C_SDA)) b |= (1 << i);
        scl_lo(); i2c_delay();
    }
    if (ack) sda_lo(); else sda_hi();
    scl_hi(); i2c_delay(); scl_lo();
    sda_hi();
    return b;
}

static void mcp_write(uint8_t reg, uint8_t val) {
    i2c_start();
    i2c_write_byte((MCP_ADDR << 1) | 0);
    i2c_write_byte(reg);
    i2c_write_byte(val);
    i2c_stop();
}

static uint8_t mcp_read(uint8_t reg) {
    i2c_start();
    i2c_write_byte((MCP_ADDR << 1) | 0);
    i2c_write_byte(reg);
    i2c_start();
    i2c_write_byte((MCP_ADDR << 1) | 1);
    uint8_t val = i2c_read_byte(false);
    i2c_stop();
    return val;
}

static uint8_t mcp_read_gpioa_stable(void) {
    uint8_t a, b;
    do {
        a = mcp_read(MCP_GPIOA);
        _delay_us(200);
        b = mcp_read(MCP_GPIOA);
    } while (a != b);
    return a;
}

void matrix_init_custom(void) {
    mcp_write(MCP_IODIRA, 0xFF);
    mcp_write(MCP_IODIRB, 0x00);
    mcp_write(MCP_GPPUA,  0xFF);
    mcp_write(MCP_OLATB,  0xFF);

    gpio_set_pin_input_high(E6); gpio_set_pin_input_high(F0); gpio_set_pin_input_high(F1);
    gpio_set_pin_input_high(F4); gpio_set_pin_input_high(F5); gpio_set_pin_input_high(F6);
    gpio_set_pin_input_high(F7); gpio_set_pin_input_high(D4);

    gpio_set_pin_output(C7); gpio_set_pin_output(C6); gpio_set_pin_output(B4);
    gpio_set_pin_output(D7); gpio_set_pin_output(D6);
    gpio_write_pin_high(C7); gpio_write_pin_high(C6); gpio_write_pin_high(B4);
    gpio_write_pin_high(D7); gpio_write_pin_high(D6);
}

bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    bool changed = false;

    // 左手側 (MCP23017): Row駆動→Col読み取り
    for (uint8_t row = 0; row < 5; row++) {
        mcp_write(MCP_OLATB, ~(1 << row));
        _delay_us(10);
        uint8_t gpioa = mcp_read_gpioa_stable();
        // GPA0=col6, GPA6=col0 なので7ビット反転
        uint8_t reversed = 0;
        for (int i = 0; i < 7; i++) {
            if (gpioa & (1 << i)) reversed |= (1 << (6 - i));
        }
        matrix_row_t left_val = ~reversed & 0x7F;
        // matrix_row_t left_val = ~gpioa & 0x7F;
        matrix_row_t new_val = left_val | (current_matrix[row] & 0xFF80);
        if (current_matrix[row] != new_val) {
            current_matrix[row] = new_val;
            changed = true;
        }
    }
    mcp_write(MCP_OLATB, 0xFF);

    // 右手側 (GPIO): Row駆動→Col読み取り
    static const pin_t right_rows[] = {C7, C6, B4, D7, D6};
    static const pin_t right_cols[] = {E6, F0, F1, F4, F5, F6, F7, D4};
    for (uint8_t row = 0; row < 5; row++) {
        gpio_write_pin_low(right_rows[row]);
        _delay_us(10);
        matrix_row_t right_val1 = 0;
        matrix_row_t right_val2 = 0;
        for (uint8_t col = 0; col < 8; col++) {
            if (!gpio_read_pin(right_cols[col])) right_val1 |= (matrix_row_t)1 << (col + 7);
        }
        _delay_us(200);
        for (uint8_t col = 0; col < 8; col++) {
            if (!gpio_read_pin(right_cols[col])) right_val2 |= (matrix_row_t)1 << (col + 7);
        }
        gpio_write_pin_high(right_rows[row]);
        // 2回一致した値のみ採用
        matrix_row_t right_val = right_val1 & right_val2;
        matrix_row_t new_val = (current_matrix[row] & 0x7F) | right_val;
        if (current_matrix[row] != new_val) {
            current_matrix[row] = new_val;
            changed = true;
        }
    }

    // --- ここにPWM維持処理を追加 ---
    // 1. ピンを出力に固定
    DDRB |= (1 << DDB5) | (1 << DDB6);
    // 2. Timer1をPWMモードに強制（スキャンによる干渉を上書き）
    TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM10);
    TCCR1B = (1 << WGM12) | (1 << CS11);
    // 3. OCR1A (PB5) の制御
    if (caps_duty > 0) {
        TCCR1A |= (1 << COM1A1); // PWM接続
        OCR1A = caps_duty;
    } else {
        TCCR1A &= ~(1 << COM1A1); // PWM切断 (単純なGPIOへ)
        PORTB &= ~(1 << PB5);    // 強制LOW
    }

    // 4. OCR1B (PB6) の制御
    if (layer_duty > 0) {
        TCCR1A |= (1 << COM1B1); // PWM接続
        OCR1B = layer_duty;
    } else {
        TCCR1A &= ~(1 << COM1B1); // PWM切断
        PORTB &= ~(1 << PB6);    // 強制LOW
    }

    return changed;
}

// CapsLock状態の監視
bool led_update_kb(led_t led_state) {
    caps_duty = led_state.caps_lock ? 64 : 0;
    return led_update_user(led_state);
}

// レイヤー状態の監視
layer_state_t layer_state_set_kb(layer_state_t state) {
    state = layer_state_set_user(state);
    layer_duty = (get_highest_layer(state) == 2) ? 64 : 0;
    return state;
}
