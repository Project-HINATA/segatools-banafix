#pragma once

#include <windows.h>

#include "swdcio/swdcio.h"

struct swdc_dll {
    uint16_t api_version;
    HRESULT (*init)(void);
    void (*get_opbtns)(uint8_t *opbtn);
    void (*get_gamebtns)(uint16_t *gamebtn);
    void (*get_analogs)(struct swdc_io_analog_state *out);
};

struct swdc_dll_config {
    wchar_t path[MAX_PATH];
};

extern struct swdc_dll swdc_dll;

HRESULT swdc_dll_init(const struct swdc_dll_config *cfg, HINSTANCE self);
