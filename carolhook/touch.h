#pragma once
#include <windows.h>

#include <stdbool.h>
#include <stdint.h>

#pragma pack(push, 1)

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

struct touch_report {
    uint8_t status;
    uint8_t touch_id;
    uint16_t x;
    uint16_t y;
};

struct touch_auto_resp {
    uint8_t rep_id;
    struct touch_report touches[10];
    uint8_t count;
    uint16_t scan_time;    
    //uint8_t padding[456];
};

#pragma pack(pop)

HRESULT touch_hook_init(const struct touch_config *cfg);