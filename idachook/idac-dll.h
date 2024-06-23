#pragma once

#include <windows.h>

#include "idacio/idacio.h"

struct idac_dll {
    uint16_t api_version;
    HRESULT (*init)(void);
    void (*get_opbtns)(uint8_t *opbtn);
    void (*get_gamebtns)(uint8_t *gamebtn);
    void (*get_shifter)(uint8_t *gear);
    void (*get_analogs)(struct idac_io_analog_state *out);
    HRESULT (*led_init)(void);
    void (*led_set_fet_output)(const uint8_t *rgb);
    void (*led_gs_update)(const uint8_t *rgb);
    void (*led_set_leds)(const uint8_t *rgb);
};

struct idac_dll_config {
    wchar_t path[MAX_PATH];
};

extern struct idac_dll idac_dll;

HRESULT idac_dll_init(const struct idac_dll_config *cfg, HINSTANCE self);
