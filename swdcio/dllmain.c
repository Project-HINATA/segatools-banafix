#include <windows.h>

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "swdcio/backend.h"
#include "swdcio/config.h"
#include "swdcio/di.h"
#include "swdcio/swdcio.h"
#include "swdcio/xi.h"

#include "util/dprintf.h"
#include "util/str.h"

static struct swdc_io_config swdc_io_cfg;
static const struct swdc_io_backend *swdc_io_backend;
static bool swdc_io_coin;

uint16_t swdc_io_get_api_version(void)
{
    return 0x0100;
}

HRESULT swdc_io_init(void)
{
    HINSTANCE inst;
    HRESULT hr;

    assert(swdc_io_backend == NULL);

    inst = GetModuleHandleW(NULL);

    if (inst == NULL) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        dprintf("GetModuleHandleW failed: %lx\n", hr);

        return hr;
    }

    swdc_io_config_load(&swdc_io_cfg, L".\\segatools.ini");

    if (wstr_ieq(swdc_io_cfg.mode, L"dinput")) {
        hr = swdc_di_init(&swdc_io_cfg.di, inst, &swdc_io_backend);
    } else if (wstr_ieq(swdc_io_cfg.mode, L"xinput")) {
        hr = swdc_xi_init(&swdc_io_cfg.xi, &swdc_io_backend);
    } else {
        hr = E_INVALIDARG;
        dprintf("swdc IO: Invalid IO mode \"%S\", use dinput or xinput\n",
                swdc_io_cfg.mode);
    }

    return hr;
}

void swdc_io_get_opbtns(uint8_t *opbtn_out)
{
    uint8_t opbtn;

    assert(swdc_io_backend != NULL);
    assert(opbtn_out != NULL);

    opbtn = 0;

    if (GetAsyncKeyState(swdc_io_cfg.vk_test) & 0x8000) {
        opbtn |= SWDC_IO_OPBTN_TEST;
    }

    if (GetAsyncKeyState(swdc_io_cfg.vk_service) & 0x8000) {
        opbtn |= SWDC_IO_OPBTN_SERVICE;
    }

    if (GetAsyncKeyState(swdc_io_cfg.vk_coin) & 0x8000) {
        if (!swdc_io_coin) {
            swdc_io_coin = true;
            opbtn |= SWDC_IO_OPBTN_COIN;
        }
    } else {
        swdc_io_coin = false;
    }

    *opbtn_out = opbtn;
}


void swdc_io_get_gamebtns(uint16_t *gamebtn_out)
{
    assert(swdc_io_backend != NULL);
    assert(gamebtn_out != NULL);

    swdc_io_backend->get_gamebtns(gamebtn_out);
}

void swdc_io_get_analogs(struct swdc_io_analog_state *out)
{
    struct swdc_io_analog_state tmp;

    assert(out != NULL);
    assert(swdc_io_backend != NULL);

    swdc_io_backend->get_analogs(&tmp);

    /* Apply steering wheel restriction. Real cabs only report about 77% of
       the IO-3's max ADC output value when the wheel is turned to either of
       its maximum positions. To match this behavior we set the default value
       for the wheel restriction config parameter to 97 (out of 128). This
       scaling factor is applied using fixed-point arithmetic below. */

    out->wheel = (tmp.wheel * swdc_io_cfg.restrict_) / 128;
    out->accel = tmp.accel;
    out->brake = tmp.brake;
}
