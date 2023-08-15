#include <windows.h>

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include "swdcio/config.h"

void swdc_di_config_load(struct swdc_di_config *cfg, const wchar_t *filename)
{
    wchar_t key[8];
    int i;

    assert(cfg != NULL);
    assert(filename != NULL);

    GetPrivateProfileStringW(
            L"dinput",
            L"deviceName",
            L"",
            cfg->device_name,
            _countof(cfg->device_name),
            filename);

    GetPrivateProfileStringW(
            L"dinput",
            L"brakeAxis",
            L"RZ",
            cfg->brake_axis,
            _countof(cfg->brake_axis),
            filename);

    GetPrivateProfileStringW(
            L"dinput",
            L"accelAxis",
            L"Y",
            cfg->accel_axis,
            _countof(cfg->accel_axis),
            filename);

    cfg->start = GetPrivateProfileIntW(L"dinput", L"start", 0, filename);
    cfg->view_chg = GetPrivateProfileIntW(L"dinput", L"viewChg", 0, filename);
    cfg->shift_dn = GetPrivateProfileIntW(L"dinput", L"shiftDn", 0, filename);
    cfg->shift_up = GetPrivateProfileIntW(L"dinput", L"shiftUp", 0, filename);
    cfg->wheel_green = GetPrivateProfileIntW(L"dinput", L"wheelGreen", 0, filename);
    cfg->wheel_red = GetPrivateProfileIntW(L"dinput", L"wheelRed", 0, filename);
    cfg->wheel_blue = GetPrivateProfileIntW(L"dinput", L"wheelBlue", 0, filename);
    cfg->wheel_yellow = GetPrivateProfileIntW(L"dinput", L"wheelYellow", 0, filename);

    cfg->reverse_brake_axis = GetPrivateProfileIntW(
                            L"dinput",
                            L"reverseBrakeAxis",
                            0,
                            filename);
    cfg->reverse_accel_axis = GetPrivateProfileIntW(
                            L"dinput",
                            L"reverseAccelAxis",
                            0,
                            filename);
}

void swdc_xi_config_load(struct swdc_xi_config *cfg, const wchar_t *filename)
{
    assert(cfg != NULL);
    assert(filename != NULL);

    cfg->single_stick_steering = GetPrivateProfileIntW(
                                L"io4",
                                L"singleStickSteering",
                                0,
                                filename);

    cfg->linear_steering = GetPrivateProfileIntW(
                           L"io4",
                           L"linearSteering",
                           0,
                           filename);
}

void swdc_io_config_load(struct swdc_io_config *cfg, const wchar_t *filename)
{
    assert(cfg != NULL);
    assert(filename != NULL);

    cfg->vk_test = GetPrivateProfileIntW(L"io4", L"test", '1', filename);
    cfg->vk_service = GetPrivateProfileIntW(L"io4", L"service", '2', filename);
    cfg->vk_coin = GetPrivateProfileIntW(L"io4", L"coin", '3', filename);
    cfg->restrict_ = GetPrivateProfileIntW(L"io4", L"restrict", 97, filename);

    GetPrivateProfileStringW(
            L"io4",
            L"mode",
            L"xinput",
            cfg->mode,
            _countof(cfg->mode),
            filename);

    swdc_shifter_config_load(&cfg->shifter, filename);
    swdc_di_config_load(&cfg->di, filename);
    swdc_xi_config_load(&cfg->xi, filename);
}

void swdc_shifter_config_load(
        struct swdc_shifter_config *cfg,
        const wchar_t *filename)
{
    assert(cfg != NULL);
    assert(filename != NULL);

    cfg->auto_neutral = GetPrivateProfileIntW(
            L"io4",
            L"autoNeutral",
            0,
            filename);
}
