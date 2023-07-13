#include <windows.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "board/io4.h"

#include "idachook/idac-dll.h"

#include "util/dprintf.h"

static HRESULT idac_io4_poll(void *ctx, struct io4_state *state);
static uint16_t coins;

static const struct io4_ops idac_io4_ops = {
    .poll = idac_io4_poll,
};

static const uint16_t idac_gear_signals[] = {
    /* Neutral */
    0x0000,
    /* 1: Left|Up */
    0x0028,
    /* 2: Left|Down */
    0x0018,
    /* 3: Up */
    0x0020,
    /* 4: Down */
    0x0010,
    /* 5: Right|Up */
    0x0024,
    /* 6: Right|Down */
    0x0014,
};

HRESULT idac_io4_hook_init(const struct io4_config *cfg)
{
    HRESULT hr;

    assert(idac_dll.init != NULL);

    hr = io4_hook_init(cfg, &idac_io4_ops, NULL);

    if (FAILED(hr)) {
        return hr;
    }

    return idac_dll.init();
}

static HRESULT idac_io4_poll(void *ctx, struct io4_state *state)
{
    uint8_t opbtn;
    uint8_t gamebtn;
    uint8_t gear;
    struct idac_io_analog_state analog_state;
    HRESULT hr;

    assert(idac_dll.poll != NULL);
    assert(idac_dll.get_opbtns != NULL);
    assert(idac_dll.get_gamebtns != NULL);
    assert(idac_dll.get_analogs != NULL);
    assert(idac_dll.get_shifter != NULL);

    memset(state, 0, sizeof(*state));
    memset(&analog_state, 0, sizeof(analog_state));

    hr = idac_dll.poll();

    if (FAILED(hr)) {
        return hr;
    }

    opbtn = 0;
    gamebtn = 0;
    gear = 0;

    idac_dll.get_opbtns(&opbtn);
    idac_dll.get_gamebtns(&gamebtn);
    idac_dll.get_shifter(&gear);
    idac_dll.get_analogs(&analog_state);

    if (opbtn & IDAC_IO_OPBTN_TEST) {
        state->buttons[0] |= IO4_BUTTON_TEST;
    }

    if (opbtn & IDAC_IO_OPBTN_SERVICE) {
        state->buttons[0] |= IO4_BUTTON_SERVICE;
    }

    if (opbtn & IDAC_IO_OPBTN_COIN) {
        coins++;
    }
    state->chutes[0] = coins << 8;

    if (gamebtn & IDAC_IO_GAMEBTN_START) {
        state->buttons[0] |= 1 << 7;
    }

    if (gamebtn & IDAC_IO_GAMEBTN_VIEW_CHANGE) {
        state->buttons[0] |= 1 << 1;
    }

    if (gamebtn & IDAC_IO_GAMEBTN_UP) {
        state->buttons[0] |= 1 << 5;
    }

    if (gamebtn & IDAC_IO_GAMEBTN_DOWN) {
        state->buttons[0] |= 1 << 4;
    }

    if (gamebtn & IDAC_IO_GAMEBTN_LEFT) {
        state->buttons[0] |= 1 << 3;
    }

    if (gamebtn & IDAC_IO_GAMEBTN_RIGHT) {
        state->buttons[0] |= 1 << 2;
    }

    /* Update simulated six-speed shifter */

    if (gear > 6) {
        gear = 6;
    }

    state->buttons[1] = idac_gear_signals[gear];

    /* Steering wheel increases left-to-right.

       Use 0x8000 as the center point. */

    state->adcs[0] = 0x8000 + analog_state.wheel;
    state->adcs[1] = analog_state.accel;
    state->adcs[2] = analog_state.brake;

    return S_OK;
}
