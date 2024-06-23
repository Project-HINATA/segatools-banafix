#pragma once

#include <windows.h>

#include <stdbool.h>
#include <stdint.h>

struct led15070_config {
    bool enable;
    unsigned int port_no;
    char board_number[8];
    uint8_t fw_ver;
    uint16_t fw_sum;
    wchar_t eeprom_path[MAX_PATH];
};

typedef HRESULT (*io_led_init_t)(void);
typedef void (*io_led_set_fet_output_t)(const uint8_t *rgb);
typedef void (*io_led_dc_update_t)(const uint8_t *rgb);
typedef void (*io_led_gs_update_t)(const uint8_t *rgb);

HRESULT led15070_hook_init(
        const struct led15070_config *cfg,
        io_led_init_t _led_init,
        io_led_set_fet_output_t _led_set_fet_output,
        io_led_dc_update_t _led_dc_update,
        io_led_gs_update_t _led_gs_update,
        unsigned int first_port,
        unsigned int num_boards);
