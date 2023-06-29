#pragma once

#include <windows.h>

#include "idacio/idacio.h"

struct idac_dll {
    uint16_t api_version;
    HRESULT (*init)(void);
    HRESULT (*poll)(void);
    void (*get_opbtns)(uint8_t *opbtn);
    void (*get_gamebtns)(uint8_t *gamebtn);
    void (*get_shifter)(uint8_t *gear);
    void (*get_analogs)(struct idac_io_analog_state *out);
};

struct idac_dll_config {
    wchar_t path[MAX_PATH];
};

extern struct idac_dll idac_dll;

HRESULT idac_dll_init(const struct idac_dll_config *cfg, HINSTANCE self);
