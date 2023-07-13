#include <windows.h>
#include <xinput.h>

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "swdcio/backend.h"
#include "swdcio/config.h"
#include "swdcio/swdcio.h"
#include "swdcio/xi.h"

#include "util/dprintf.h"

static void swdc_xi_get_gamebtns(uint16_t *gamebtn_out);
static void swdc_xi_get_analogs(struct swdc_io_analog_state *out);

static HRESULT swdc_xi_config_apply(const struct swdc_xi_config *cfg);

static const struct swdc_io_backend swdc_xi_backend = {
    .get_gamebtns  = swdc_xi_get_gamebtns,
    .get_analogs   = swdc_xi_get_analogs,
};

static bool swdc_xi_single_stick_steering;

HRESULT swdc_xi_init(const struct swdc_xi_config *cfg, const struct swdc_io_backend **backend)
{
    HRESULT hr;
    assert(cfg != NULL);
    assert(backend != NULL);

    hr = swdc_xi_config_apply(cfg);

    if (FAILED(hr)) {
        return hr;
    }

    dprintf("XInput: Using XInput controller\n");
    *backend = &swdc_xi_backend;

    return S_OK;
}

HRESULT swdc_io_poll(void)
{    
    return S_OK;
}

static HRESULT swdc_xi_config_apply(const struct swdc_xi_config *cfg)
{
    dprintf("XInput: --- Begin configuration ---\n");
    dprintf("XInput: Single Stick Steering : %i\n", cfg->single_stick_steering);
    dprintf("XInput: ---  End  configuration ---\n");

    swdc_xi_single_stick_steering = cfg->single_stick_steering;

    return S_OK;
}

static void swdc_xi_get_gamebtns(uint16_t *gamebtn_out)
{
    uint16_t gamebtn;
    XINPUT_STATE xi;
    WORD xb;

    assert(gamebtn_out != NULL);

    gamebtn = 0;

    memset(&xi, 0, sizeof(xi));
    XInputGetState(0, &xi);
    xb = xi.Gamepad.wButtons;

    if (xb & XINPUT_GAMEPAD_DPAD_UP) {
        gamebtn |= SWDC_IO_GAMEBTN_UP;
    }

    if (xb & XINPUT_GAMEPAD_DPAD_DOWN) {
        gamebtn |= SWDC_IO_GAMEBTN_DOWN;
    }

    if (xb & XINPUT_GAMEPAD_DPAD_LEFT) {
        gamebtn |= SWDC_IO_GAMEBTN_LEFT;
    }

    if (xb & XINPUT_GAMEPAD_DPAD_RIGHT) {
        gamebtn |= SWDC_IO_GAMEBTN_RIGHT;
    }

    if (xb & XINPUT_GAMEPAD_START) {
        gamebtn |= SWDC_IO_GAMEBTN_START;
    }

    if (xb & XINPUT_GAMEPAD_BACK) {
        gamebtn |= SWDC_IO_GAMEBTN_VIEW_CHANGE;
    }

    if (xb & XINPUT_GAMEPAD_A) {
        gamebtn |= SWDC_IO_GAMEBTN_STEERING_GREEN;
    }

    if (xb & XINPUT_GAMEPAD_B) {
        gamebtn |= SWDC_IO_GAMEBTN_STEERING_RED;
    }

    if (xb & XINPUT_GAMEPAD_X) {
        gamebtn |= SWDC_IO_GAMEBTN_STEERING_BLUE;
    }

    if (xb & XINPUT_GAMEPAD_Y) {
        gamebtn |= SWDC_IO_GAMEBTN_STEERING_YELLOW;
    }

    if (xb & XINPUT_GAMEPAD_LEFT_SHOULDER) {
        gamebtn |= SWDC_IO_GAMEBTN_STEERING_PADDLE_LEFT;
    }

    if (xb & XINPUT_GAMEPAD_RIGHT_SHOULDER) {
        gamebtn |= SWDC_IO_GAMEBTN_STEERING_PADDLE_RIGHT;
    }

    *gamebtn_out = gamebtn;
}

static void swdc_xi_get_analogs(struct swdc_io_analog_state *out)
{
    XINPUT_STATE xi;
    int left;
    int right;

    assert(out != NULL);

    memset(&xi, 0, sizeof(xi));
    XInputGetState(0, &xi);

    left = xi.Gamepad.sThumbLX;

    if (left < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) {
        left += XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
    } else if (left > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) {
        left -= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
    } else {
        left = 0;
    }

    right = xi.Gamepad.sThumbRX;

    if (right < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) {
        right += XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
    } else if (right > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) {
        right -= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
    } else {
        right = 0;
    }

    if (swdc_xi_single_stick_steering) {
        out->wheel = left;
    } else {
        out->wheel = (left + right) / 2;
    }

    out->accel = xi.Gamepad.bRightTrigger << 8;
    out->brake = xi.Gamepad.bLeftTrigger << 8;
}
