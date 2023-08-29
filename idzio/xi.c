#include <windows.h>
#include <xinput.h>

#include <math.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "idzio/backend.h"
#include "idzio/config.h"
#include "idzio/idzio.h"
#include "idzio/shifter.h"
#include "idzio/xi.h"

#include "util/dprintf.h"

static void idz_xi_jvs_read_buttons(uint8_t *gamebtn_out);
static void idz_xi_jvs_read_shifter(uint8_t *gear);
static void idz_xi_jvs_read_analogs(struct idz_io_analog_state *out);

static HRESULT idz_xi_config_apply(const struct idz_xi_config *cfg);

static const struct idz_io_backend idz_xi_backend = {
    .jvs_read_buttons   = idz_xi_jvs_read_buttons,
    .jvs_read_shifter   = idz_xi_jvs_read_shifter,
    .jvs_read_analogs   = idz_xi_jvs_read_analogs,
};

static bool idz_xi_single_stick_steering;
static bool idz_xi_linear_steering;
static uint16_t idz_xi_left_stick_deadzone;
static uint16_t idz_xi_right_stick_deadzone;

HRESULT idz_xi_init(const struct idz_xi_config *cfg, const struct idz_io_backend **backend)
{
    HRESULT hr;
    assert(cfg != NULL);
    assert(backend != NULL);

    hr = idz_xi_config_apply(cfg);

    if (FAILED(hr)) {
        return hr;
    }

    dprintf("XInput: Using XInput controller\n");
    *backend = &idz_xi_backend;

    return S_OK;
}

static HRESULT idz_xi_config_apply(const struct idz_xi_config *cfg) {
    /* Deadzones check */
    if (cfg->left_stick_deadzone > 32767 || cfg->left_stick_deadzone < 0) {
        dprintf("XInput: Left stick deadzone is too large or negative\n");
        return E_INVALIDARG;
    }

    if (cfg->right_stick_deadzone > 32767 || cfg->right_stick_deadzone < 0) {
        dprintf("XInput: Right stick deadzone is too large or negative\n");
        return E_INVALIDARG;
    }

    dprintf("XInput: --- Begin configuration ---\n");
    dprintf("XInput: Single Stick Steering : %i\n", cfg->single_stick_steering);
    dprintf("XInput: Linear Steering . . . : %i\n", cfg->linear_steering);
    dprintf("XInput: Left Deadzone . . . . : %i\n", cfg->left_stick_deadzone);
    dprintf("XInput: Right Deadzone  . . . : %i\n", cfg->right_stick_deadzone);
    dprintf("XInput: ---  End  configuration ---\n");

    idz_xi_single_stick_steering = cfg->single_stick_steering;
    idz_xi_linear_steering = cfg->linear_steering;
    idz_xi_left_stick_deadzone = cfg->left_stick_deadzone;
    idz_xi_right_stick_deadzone = cfg->right_stick_deadzone;

    return S_OK;
}

static void idz_xi_jvs_read_buttons(uint8_t *gamebtn_out)
{
    uint8_t gamebtn;
    XINPUT_STATE xi;
    WORD xb;

    assert(gamebtn_out != NULL);

    gamebtn = 0;

    memset(&xi, 0, sizeof(xi));
    XInputGetState(0, &xi);
    xb = xi.Gamepad.wButtons;

    if (xb & XINPUT_GAMEPAD_DPAD_UP) {
        gamebtn |= IDZ_IO_GAMEBTN_UP;
    }

    if (xb & XINPUT_GAMEPAD_DPAD_DOWN) {
        gamebtn |= IDZ_IO_GAMEBTN_DOWN;
    }

    if (xb & XINPUT_GAMEPAD_DPAD_LEFT) {
        gamebtn |= IDZ_IO_GAMEBTN_LEFT;
    }

    if (xb & XINPUT_GAMEPAD_DPAD_RIGHT) {
        gamebtn |= IDZ_IO_GAMEBTN_RIGHT;
    }

    if (xb & (XINPUT_GAMEPAD_START | XINPUT_GAMEPAD_A)) {
        gamebtn |= IDZ_IO_GAMEBTN_START;
    }

    if (xb & (XINPUT_GAMEPAD_BACK | XINPUT_GAMEPAD_B)) {
        gamebtn |= IDZ_IO_GAMEBTN_VIEW_CHANGE;
    }

    *gamebtn_out = gamebtn;
}

static void idz_xi_jvs_read_shifter(uint8_t *gear)
{
    bool shift_dn;
    bool shift_up;
    XINPUT_STATE xi;
    WORD xb;

    assert(gear != NULL);

    memset(&xi, 0, sizeof(xi));
    XInputGetState(0, &xi);
    xb = xi.Gamepad.wButtons;

    if (xb & XINPUT_GAMEPAD_START) {
        /* Reset to Neutral when start is pressed */
        idz_shifter_reset();
    }

    shift_dn = xb & (XINPUT_GAMEPAD_Y | XINPUT_GAMEPAD_LEFT_SHOULDER);
    shift_up = xb & (XINPUT_GAMEPAD_X | XINPUT_GAMEPAD_RIGHT_SHOULDER);

    idz_shifter_update(shift_dn, shift_up);

    *gear = idz_shifter_current_gear();
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

static void idz_xi_jvs_read_analogs(struct idz_io_analog_state *out)
{
    XINPUT_STATE xi;
    int left;
    int right;

    assert(out != NULL);

    memset(&xi, 0, sizeof(xi));
    XInputGetState(0, &xi);

    left = xi.Gamepad.sThumbLX;
    right = xi.Gamepad.sThumbRX;
    
    if (!idz_xi_linear_steering) {
        // Apply non-linear transform for both sticks
        left = apply_non_linear_transform(left, idz_xi_left_stick_deadzone);
        right = apply_non_linear_transform(right, idz_xi_right_stick_deadzone);
    } else {
        if (left < -idz_xi_left_stick_deadzone) {
            left += idz_xi_left_stick_deadzone;
        } else if (left > idz_xi_left_stick_deadzone) {
            left -= idz_xi_left_stick_deadzone;
        } else {
            left = 0;
        }

        if (right < -idz_xi_right_stick_deadzone) {
            right += idz_xi_right_stick_deadzone;
        } else if (right > idz_xi_right_stick_deadzone) {
            right -= idz_xi_right_stick_deadzone;
        } else {
            right = 0;
        }
    }

    if(idz_xi_single_stick_steering) {
        out->wheel = left;
    } else {
        out->wheel = (left + right) / 2;
    }

    out->accel = xi.Gamepad.bRightTrigger << 8;
    out->brake = xi.Gamepad.bLeftTrigger << 8;
}
