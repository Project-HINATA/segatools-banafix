#pragma once

#include <windows.h>

#include <stdbool.h>
#include <stdint.h>

struct led15093_config {
    bool enable;
    bool high_baudrate;
    unsigned int port_no;
    char board_number[8];
    char chip_number[5];
    char boot_chip_number[5];
    uint8_t fw_ver;
    uint16_t fw_sum;
};

typedef int (*io_led_init_t)();
typedef void (*io_led_set_leds_t)(uint8_t board, uint8_t *rgb);

HRESULT led15093_hook_init(const struct led15093_config *cfg, io_led_init_t _led_init, 
    io_led_set_leds_t _set_leds, unsigned int first_port, unsigned int num_boards, uint8_t board_adr, uint8_t host_adr);

