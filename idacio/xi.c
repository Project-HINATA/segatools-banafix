#include "idacio/xi.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <windows.h>
#include <xinput.h>

#include "idacio/backend.h"
#include "idacio/config.h"
#include "idacio/idacio.h"
#include "idacio/shifter.h"
#include "util/dprintf.h"

static void idac_xi_get_gamebtns(uint8_t *gamebtn_out);
static void idac_xi_get_shifter(uint8_t *gear);
static void idac_xi_get_analogs(struct idac_io_analog_state *out);

static HRESULT idac_xi_config_apply(const struct idac_xi_config *cfg);

static const struct idac_io_backend idac_xi_backend = {
    .get_gamebtns = idac_xi_get_gamebtns,
    .get_shifter = idac_xi_get_shifter,
    .get_analogs = idac_xi_get_analogs,
};

static bool idac_xi_single_stick_steering;
static bool idac_xi_linear_steering;
static uint16_t idac_xi_left_stick_deadzone;
static uint16_t idac_xi_right_stick_deadzone;

HRESULT idac_xi_init(const struct idac_xi_config *cfg, const struct idac_io_backend **backend) {
    HRESULT hr;
    assert(cfg != NULL);
    assert(backend != NULL);

    hr = idac_xi_config_apply(cfg);

    if (FAILED(hr)) {
        return hr;
    }

    dprintf("XInput: Using XInput controller\n");
    *backend = &idac_xi_backend;

    return S_OK;
}

HRESULT idac_io_poll(void) {
    return S_OK;
}

static HRESULT idac_xi_config_apply(const struct idac_xi_config *cfg) {
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

    idac_xi_single_stick_steering = cfg->single_stick_steering;
    idac_xi_linear_steering = cfg->linear_steering;
    idac_xi_left_stick_deadzone = cfg->left_stick_deadzone;
    idac_xi_right_stick_deadzone = cfg->right_stick_deadzone;

    return S_OK;
}

static void idac_xi_get_gamebtns(uint8_t *gamebtn_out) {
    uint8_t gamebtn;
    XINPUT_STATE xi;
    WORD xb;

    assert(gamebtn_out != NULL);

    gamebtn = 0;

    memset(&xi, 0, sizeof(xi));
    XInputGetState(0, &xi);
    xb = xi.Gamepad.wButtons;

    if (xb & XINPUT_GAMEPAD_DPAD_UP) {
        gamebtn |= IDAC_IO_GAMEBTN_UP;
    }

    if (xb & XINPUT_GAMEPAD_DPAD_DOWN) {
        gamebtn |= IDAC_IO_GAMEBTN_DOWN;
    }

    if (xb & (XINPUT_GAMEPAD_DPAD_LEFT | XINPUT_GAMEPAD_LEFT_THUMB)) {
        gamebtn |= IDAC_IO_GAMEBTN_LEFT;
    }

    if (xb & (XINPUT_GAMEPAD_DPAD_RIGHT | XINPUT_GAMEPAD_RIGHT_THUMB)) {
        gamebtn |= IDAC_IO_GAMEBTN_RIGHT;
    }

    if (xb & (XINPUT_GAMEPAD_START | XINPUT_GAMEPAD_A)) {
        gamebtn |= IDAC_IO_GAMEBTN_START;
    }

    if (xb & (XINPUT_GAMEPAD_BACK | XINPUT_GAMEPAD_B)) {
        gamebtn |= IDAC_IO_GAMEBTN_VIEW_CHANGE;
    }

    *gamebtn_out = gamebtn;
}

static void idac_xi_get_shifter(uint8_t *gear) {
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
        idac_shifter_set(0);
    }

    shift_dn = xb & (XINPUT_GAMEPAD_Y | XINPUT_GAMEPAD_LEFT_SHOULDER);
    shift_up = xb & (XINPUT_GAMEPAD_X | XINPUT_GAMEPAD_RIGHT_SHOULDER);

    idac_shifter_update(shift_dn, shift_up);

    *gear = idac_shifter_current_gear();
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

static void idac_xi_get_analogs(struct idac_io_analog_state *out) {
    XINPUT_STATE xi;
    int left;
    int right;

    assert(out != NULL);

    memset(&xi, 0, sizeof(xi));
    XInputGetState(0, &xi);

    left = xi.Gamepad.sThumbLX;
    right = xi.Gamepad.sThumbRX;

    if (!idac_xi_linear_steering) {
        // Apply non-linear transform for both sticks
        left = apply_non_linear_transform(left, idac_xi_left_stick_deadzone);
        right = apply_non_linear_transform(right, idac_xi_right_stick_deadzone);
    } else {
        if (left < -idac_xi_left_stick_deadzone) {
            left += idac_xi_left_stick_deadzone;
        } else if (left > idac_xi_left_stick_deadzone) {
            left -= idac_xi_left_stick_deadzone;
        } else {
            left = 0;
        }

        if (right < -idac_xi_right_stick_deadzone) {
            right += idac_xi_right_stick_deadzone;
        } else if (right > idac_xi_right_stick_deadzone) {
            right -= idac_xi_right_stick_deadzone;
        } else {
            right = 0;
        }
    }

    if (idac_xi_single_stick_steering) {
        out->wheel = left;
    } else {
        out->wheel = (left + right) / 2;
    }

    out->accel = xi.Gamepad.bRightTrigger << 8;
    out->brake = xi.Gamepad.bLeftTrigger << 8;
}
