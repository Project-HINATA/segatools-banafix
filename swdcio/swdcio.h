#pragma once

#include <windows.h>

#include <stdint.h>

enum {
    SWDC_IO_OPBTN_TEST = 0x01,
    SWDC_IO_OPBTN_SERVICE = 0x02,
    SWDC_IO_OPBTN_COIN = 0x04,
};

enum {
    SWDC_IO_GAMEBTN_UP = 0x01,
    SWDC_IO_GAMEBTN_DOWN = 0x02,
    SWDC_IO_GAMEBTN_LEFT = 0x04,
    SWDC_IO_GAMEBTN_RIGHT = 0x08,
    SWDC_IO_GAMEBTN_START = 0x10,
    SWDC_IO_GAMEBTN_VIEW_CHANGE = 0x20,

    SWDC_IO_GAMEBTN_STEERING_BLUE = 0x40,
    SWDC_IO_GAMEBTN_STEERING_GREEN = 0x80,
    SWDC_IO_GAMEBTN_STEERING_RED = 0x100,
    SWDC_IO_GAMEBTN_STEERING_YELLOW = 0x200,
    SWDC_IO_GAMEBTN_STEERING_PADDLE_LEFT = 0x400,
    SWDC_IO_GAMEBTN_STEERING_PADDLE_RIGHT = 0x800,
};

struct swdc_io_analog_state {
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

uint16_t swdc_io_get_api_version(void);

/* Initialize the IO DLL. This is the second function that will be called on
   your DLL, after SWDC_io_get_api_version.

   All subsequent calls to this API may originate from arbitrary threads.

   Minimum API version: 0x0100 */

HRESULT swdc_io_init(void);

/* Get the state of the cabinet's operator buttons as of the last poll. See
   SWDC_IO_OPBTN enum above: this contains bit mask definitions for button
   states returned in *opbtn. All buttons are active-high.

   Minimum API version: 0x0100 */

void swdc_io_get_opbtns(uint8_t *opbtn);

/* Get the state of the cabinet's gameplay buttons as of the last poll. See
   SWDC_IO_GAMEBTN enum above for bit mask definitions. Inputs are split into
   a left hand side set of inputs and a right hand side set of inputs: the bit
   mappings are the same in both cases.

   All buttons are active-high, even though some buttons' electrical signals
   on a real cabinet are active-low.

   Minimum API version: 0x0100 */

void swdc_io_get_gamebtns(uint16_t *gamebtn);

/* Poll the current state of the cabinet's JVS analog inputs. See structure
   definition above for details.

   Minimum API version: 0x0100 */

void swdc_io_get_analogs(struct swdc_io_analog_state *out);
