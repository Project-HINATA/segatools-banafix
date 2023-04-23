#pragma once

#include <stdbool.h>
#include <stdint.h>

void idac_shifter_reset(void);
void idac_shifter_update(bool shift_dn, bool shift_up);
uint8_t idac_shifter_current_gear(void);
