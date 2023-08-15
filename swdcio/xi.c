#include <windows.h>
#include <xinput.h>

#include <math.h>
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
static bool swdc_xi_linear_steering;

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
    dprintf("XInput: Linear Steering . . . : %i\n", cfg->linear_steering);
    dprintf("XInput: ---  End  configuration ---\n");

    swdc_xi_single_stick_steering = cfg->single_stick_steering;
    swdc_xi_linear_steering = cfg->linear_steering;

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

static int apply_non_linear_transform(int value, int deadzone_center) {
    const int max_input = 32767;
    const double power_factor = 3.0;

    // Apply deadzone only after passing the center threshold
    if (abs(value) < deadzone_center) {
        return 0;
    }

    // Scale the value to the range [-1.0, 1.0]
    double scaled_value = (abs(value) - deadzone_center) / (double)(max_input - deadzone_center);

    // Apply a non-linear transform (cubing in this case) and preserve the sign
    double signed_value = copysign(pow(scaled_value, power_factor), value);

    // Scale the value back to the range [-32770, 32767]
    int transformed_value = (int)(signed_value * max_input);

    // Clamp the value to the range [-32767, 32767]
    transformed_value = (transformed_value > max_input) ? max_input : transformed_value;
    transformed_value = (transformed_value < -max_input) ? -max_input : transformed_value;

    return transformed_value;
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
    right = xi.Gamepad.sThumbRX;

    if (!swdc_xi_linear_steering) {
        // Apply non-linear transform for both sticks
        left = apply_non_linear_transform(left, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
        right = apply_non_linear_transform(right, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
    } else {
        if (left < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) {
            left += XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
        } else if (left > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) {
            left -= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
        } else {
            left = 0;
        }

        if (right < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) {
            right += XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
        } else if (right > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) {
            right -= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
        } else {
            right = 0;
        }
    }

    if (swdc_xi_single_stick_steering) {
        out->wheel = left;
    } else {
        out->wheel = (left + right) / 2;
    }

    out->accel = xi.Gamepad.bRightTrigger << 8;
    out->brake = xi.Gamepad.bLeftTrigger << 8;
}
