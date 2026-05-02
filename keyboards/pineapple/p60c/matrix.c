#include "matrix.h"
#include "wait.h"
#include <util/delay.h>

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

void matrix_init_custom(void) {
    // MCP23017: GPIOA=Col(入力プルアップ), GPIOB=Row(出力)
    mcp_write(MCP_IODIRA, 0xFF);   // GPIOA all input
    mcp_write(MCP_IODIRB, 0x00);   // GPIOB all output
    mcp_write(MCP_GPPUA,  0xFF);   // GPIOA pull-up ON
    mcp_write(MCP_OLATB,  0xFF);   // 全Row Hi（非選択）

    // 右手側 Col(入力プルアップ): PE6,PF0,PF1,PF4,PF5,PF6,PF7,PD4
    gpio_set_pin_input_high(E6); gpio_set_pin_input_high(F0); gpio_set_pin_input_high(F1);
    gpio_set_pin_input_high(F4); gpio_set_pin_input_high(F5); gpio_set_pin_input_high(F6);
    gpio_set_pin_input_high(F7); gpio_set_pin_input_high(D4);

    // 右手側 Row(出力): PC7,PC6,PB4,PD7,PD6
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
        uint8_t gpioa = mcp_read(MCP_GPIOA);
        matrix_row_t new_val = ~gpioa & 0x7F;
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
        matrix_row_t new_val = 0;
        for (uint8_t col = 0; col < 8; col++) {
            if (!gpio_read_pin(right_cols[col])) {
                new_val |= (matrix_row_t)1 << (col + 7);
            }
        }
        if (current_matrix[row + 5] != new_val) {
            current_matrix[row + 5] = new_val;
            changed = true;
        }
        gpio_write_pin_high(right_rows[row]);
    }

    return changed;
}
