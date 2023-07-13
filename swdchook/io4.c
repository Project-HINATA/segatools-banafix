#include <windows.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "board/io4.h"

#include "swdchook/swdc-dll.h"

#include "util/dprintf.h"

static HRESULT swdc_io4_poll(void *ctx, struct io4_state *state);
static uint16_t coins;

static const struct io4_ops swdc_io4_ops = {
    .poll = swdc_io4_poll,
};

HRESULT swdc_io4_hook_init(const struct io4_config *cfg)
{
    HRESULT hr;

    assert(swdc_dll.init != NULL);

    hr = io4_hook_init(cfg, &swdc_io4_ops, NULL);

    if (FAILED(hr)) {
        return hr;
    }

    return swdc_dll.init();
}

static HRESULT swdc_io4_poll(void *ctx, struct io4_state *state)
{
    uint8_t opbtn;
    uint16_t gamebtn;
    struct swdc_io_analog_state analog_state;
    HRESULT hr;

    assert(swdc_dll.poll != NULL);
    assert(swdc_dll.get_opbtns != NULL);
    assert(swdc_dll.get_gamebtns != NULL);
    assert(swdc_dll.get_analogs != NULL);

    memset(state, 0, sizeof(*state));
    memset(&analog_state, 0, sizeof(analog_state));

    hr = swdc_dll.poll();

    if (FAILED(hr)) {
        return hr;
    }

    opbtn = 0;
    gamebtn = 0;

    swdc_dll.get_opbtns(&opbtn);
    swdc_dll.get_gamebtns(&gamebtn);
    swdc_dll.get_analogs(&analog_state);

    if (opbtn & SWDC_IO_OPBTN_TEST) {
        state->buttons[0] |= IO4_BUTTON_TEST;
    }

    if (opbtn & SWDC_IO_OPBTN_SERVICE) {
        state->buttons[0] |= IO4_BUTTON_SERVICE;
    }

    if (opbtn & SWDC_IO_OPBTN_COIN) {
        coins++;
    }
    state->chutes[0] = coins << 8;

    /* Update Cabinet buttons */

    if (gamebtn & SWDC_IO_GAMEBTN_START) {
        state->buttons[0] |= 1 << 7;
    }

    if (gamebtn & SWDC_IO_GAMEBTN_VIEW_CHANGE) {
        state->buttons[0] |= 1 << 1;
    }

    if (gamebtn & SWDC_IO_GAMEBTN_UP) {
        state->buttons[0] |= 1 << 5;
    }

    if (gamebtn & SWDC_IO_GAMEBTN_DOWN) {
        state->buttons[0] |= 1 << 4;
    }

    if (gamebtn & SWDC_IO_GAMEBTN_LEFT) {
        state->buttons[0] |= 1 << 3;
    }

    if (gamebtn & SWDC_IO_GAMEBTN_RIGHT) {
        state->buttons[0] |= 1 << 2;
    }

    /* Update steering wheel buttons */

    if (gamebtn & SWDC_IO_GAMEBTN_STEERING_BLUE) {
        state->buttons[1] |= 1 << 15;
    }

    if (gamebtn & SWDC_IO_GAMEBTN_STEERING_GREEN) {
        state->buttons[1] |= 1 << 14;
    }

    if (gamebtn & SWDC_IO_GAMEBTN_STEERING_RED) {
        state->buttons[1] |= 1 << 13;
    }

    if (gamebtn & SWDC_IO_GAMEBTN_STEERING_YELLOW) {
        state->buttons[1] |= 1 << 12;
    }

    if (gamebtn & SWDC_IO_GAMEBTN_STEERING_PADDLE_LEFT) {
        state->buttons[1] |= 1 << 1;
    }

    if (gamebtn & SWDC_IO_GAMEBTN_STEERING_PADDLE_RIGHT) {
        state->buttons[1] |= 1 << 0;
    }

    /* Steering wheel increases left-to-right.

       Use 0x8000 as the center point. */

    state->adcs[0] = 0x8000 + analog_state.wheel;
    state->adcs[1] = analog_state.accel;
    state->adcs[2] = analog_state.brake;

    return S_OK;
}
