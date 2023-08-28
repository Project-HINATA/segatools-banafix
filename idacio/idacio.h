#pragma once

#include <windows.h>

#include <stdint.h>

enum {
    IDAC_IO_OPBTN_TEST = 0x01,
    IDAC_IO_OPBTN_SERVICE = 0x02,
    IDAC_IO_OPBTN_COIN = 0x04,
};

enum {
    IDAC_IO_GAMEBTN_UP = 0x01,
    IDAC_IO_GAMEBTN_DOWN = 0x02,
    IDAC_IO_GAMEBTN_LEFT = 0x04,
    IDAC_IO_GAMEBTN_RIGHT = 0x08,
    IDAC_IO_GAMEBTN_START = 0x10,
    IDAC_IO_GAMEBTN_VIEW_CHANGE = 0x20,
};

struct idac_io_analog_state {
    /* Current steering wheel position, where zero is the centered position.

       The game will accept any signed 16-bit position value, however a real
       cabinet will report a value of approximately +/- 25230 when the wheel
       is at full lock. Steering wheel positions of a magnitude greater than
       this value are not possible on a real cabinet. */

    int16_t wheel;

    /* Current position of the accelerator pedal, where 0 is released. */

    uint16_t accel;

    /* Current position of the brake pedal, where 0 is released. */

    uint16_t brake;
};

/* Get the version of the IDAC IO API that this DLL supports. This
   function should return a positive 16-bit integer, where the high byte is
   the major version and the low byte is the minor version (as defined by the
   Semantic Versioning standard).

   The latest API version as of this writing is 0x0100. */

uint16_t idac_io_get_api_version(void);

/* Initialize the IO DLL. This is the second function that will be called on
   your DLL, after mu3_io_get_api_version.

   All subsequent calls to this API may originate from arbitrary threads.

   Minimum API version: 0x0100 */

HRESULT idac_io_init(void);

/* Send any queued outputs (of which there are currently none, though this may
   change in subsequent API versions) and retrieve any new inputs.

   Minimum API version: 0x0100 */

HRESULT idac_io_poll(void);

/* Get the state of the cabinet's operator buttons as of the last poll. See
   IDAC_IO_OPBTN enum above: this contains bit mask definitions for button
   states returned in *opbtn. All buttons are active-high.

   Minimum API version: 0x0100 */

void idac_io_get_opbtns(uint8_t *opbtn);

/* Get the state of the cabinet's gameplay buttons as of the last poll. See
   IDAC_IO_GAMEBTN enum above for bit mask definitions. Inputs are split into
   a left hand side set of inputs and a right hand side set of inputs: the bit
   mappings are the same in both cases.

   All buttons are active-high, even though some buttons' electrical signals
   on a real cabinet are active-low.

   Minimum API version: 0x0100 */

void idac_io_get_gamebtns(uint8_t *gamebtn);

/* Poll the current state of the cabinet's IO4 analog inputs. See structure
   definition above for details.

   Minimum API version: 0x0100 */

void idac_io_get_analogs(struct idac_io_analog_state *out);

/* Poll the current position of the six-speed shifter and return it via the
   gear out parameter. Valid values are 0 for neutral and 1-6 for gears 1-6.

   idzhook internally translates this gear position value into the correct
   combination of Gear Left, Gear Right, Gear Up, Gear Down buttons that the
   game will then interpret as the current position of the gear lever.

   Minimum API version: 0x0100 */

void idac_io_get_shifter(uint8_t *gear);