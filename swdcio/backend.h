#pragma once

#include <stdint.h>

#include "swdcio/swdcio.h"

struct swdc_io_backend {
    void (*get_opbtns)(uint8_t *opbtn);
    void (*get_gamebtns)(uint16_t *gamebtn);
    void (*get_analogs)(struct swdc_io_analog_state *state);
};
