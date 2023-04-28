#pragma once
#include <windows.h>

#include <stdbool.h>
#include <stdint.h>

struct touch_config {
    bool enable;
};

// Always starts with 0x01, always ends with 0x0D
struct touch_req {
    uint8_t sync; // Always 0x01
    uint8_t cmd[256]; // rest of the data goes here
    uint8_t tail; // Always 0x0D
    size_t data_len; // length of data
};

HRESULT touch_hook_init(const struct touch_config *cfg);