#pragma once

#include <stdint.h>

#include "idacio/idacio.h"

struct idac_io_backend {
    void (*get_opbtns)(uint8_t *opbtn);
    void (*get_gamebtns)(uint8_t *gamebtn);
    void (*get_shifter)(uint8_t *gear);
    void (*get_analogs)(struct idac_io_analog_state *state);
};
