MCU = atmega32u4
BOOTLOADER = atmel-dfu

SPLIT_KEYBOARD = no
# CONSOLE_ENABLE = yes
CUSTOM_MATRIX = lite

# キー変化後に一定時間待つ方式で、チャタリングに強い。デフォルトの sym_eager_pk より効果的な場合がある
# DEBOUNCE_TYPE = sym_defer_g
# キーごとに待つので、より精密
DEBOUNCE_TYPE = sym_defer_pk

PS2_MOUSE_ENABLE = yes
PS2_DRIVER = interrupt

SRC += matrix.c