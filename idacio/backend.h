#pragma once

#include <stdint.h>

#include "idacio/idacio.h"

struct idac_io_backend {
    void (*jvs_read_buttons)(uint8_t *gamebtn);
    void (*jvs_read_shifter)(uint8_t *gear);
    void (*jvs_read_analogs)(struct idac_io_analog_state *state);
};
