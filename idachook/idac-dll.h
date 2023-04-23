#pragma once

#include <windows.h>

#include "idacio/idacio.h"

struct idac_dll {
    uint16_t api_version;
    HRESULT (*jvs_init)(void);
    void (*jvs_read_analogs)(struct idac_io_analog_state *out);
    void (*jvs_read_buttons)(uint8_t *opbtn, uint8_t *gamebtn);
    void (*jvs_read_shifter)(uint8_t *gear);
    void (*jvs_read_coin_counter)(uint16_t *total);
};

struct idac_dll_config {
    wchar_t path[MAX_PATH];
};

extern struct idac_dll idac_dll;

HRESULT idac_dll_init(const struct idac_dll_config *cfg, HINSTANCE self);
