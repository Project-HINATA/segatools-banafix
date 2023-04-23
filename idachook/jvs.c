#include <windows.h>

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "amex/jvs.h"

#include "board/io3.h"

#include "idachook/idac-dll.h"
#include "idachook/jvs.h"

#include "jvs/jvs-bus.h"

#include "util/dprintf.h"

static void idac_jvs_read_analogs(
        void *ctx,
        uint16_t *analogs,
        uint8_t nanalogs);
static void idac_jvs_read_switches(void *ctx, struct io3_switch_state *out);
static void idac_jvs_read_coin_counter(
        void *ctx,
        uint8_t slot_no,
        uint16_t *out);

static const struct io3_ops idac_jvs_io3_ops = {
    .read_switches      = idac_jvs_read_switches,
    .read_analogs       = idac_jvs_read_analogs,
    .read_coin_counter  = idac_jvs_read_coin_counter,
};

static const uint16_t idac_jvs_gear_signals[] = {
    /* Neutral */
    0x0000,
    /* 1: Left|Up */
    0x2800,
    /* 2: Left|Down */
    0x1800,
    /* 3: Up */
    0x2000,
    /* 4: Down */
    0x1000,
    /* 5: Right|Up */
    0x2400,
    /* 6: Right|Down */
    0x1400,
};

static struct io3 idac_jvs_io3;

HRESULT idac_jvs_init(struct jvs_node **out)
{
    HRESULT hr;

    assert(out != NULL);
    assert(idac_dll.jvs_init != NULL);

    dprintf("JVS I/O: Starting Initial D Zero backend DLL\n");
    hr = idac_dll.jvs_init();

    if (FAILED(hr)) {
        dprintf("JVS I/O: Backend error, I/O disconnected; %x\n", (int) hr);

        return hr;
    }

    io3_init(&idac_jvs_io3, NULL, &idac_jvs_io3_ops, NULL);
    *out = io3_to_jvs_node(&idac_jvs_io3);

    return S_OK;
}

static void idac_jvs_read_switches(void *ctx, struct io3_switch_state *out)
{
    uint8_t opbtn;
    uint8_t gamebtn;
    uint8_t gear;

    assert(out != NULL);
    assert(idac_dll.jvs_read_buttons != NULL);
    assert(idac_dll.jvs_read_shifter != NULL);

    opbtn = 0;
    gamebtn = 0;
    gear = 0;

    idac_dll.jvs_read_buttons(&opbtn, &gamebtn);
    idac_dll.jvs_read_shifter(&gear);

    /* Update gameplay buttons */

    if (gamebtn & IDAC_IO_GAMEBTN_UP) {
        out->p1 |= 1 << 13;
    }

    if (gamebtn & IDAC_IO_GAMEBTN_DOWN) {
        out->p1 |= 1 << 12;
    }

    if (gamebtn & IDAC_IO_GAMEBTN_LEFT) {
        out->p1 |= 1 << 11;
    }

    if (gamebtn & IDAC_IO_GAMEBTN_RIGHT) {
        out->p1 |= 1 << 10;
    }

    if (gamebtn & IDAC_IO_GAMEBTN_START) {
        out->p1 |= 1 << 15;
    }

    if (gamebtn & IDAC_IO_GAMEBTN_VIEW_CHANGE) {
        out->p1 |= 1 << 9;
    }

    /* Update simulated six-speed shifter */

    if (gear > 6) {
        gear = 6;
    }

    out->p2 = idac_jvs_gear_signals[gear];

    /* Update test/service buttons */

    if (opbtn & IDAC_IO_OPBTN_TEST) {
        out->system = 0x80;
    } else {
        out->system = 0;
    }

    if (opbtn & IDAC_IO_OPBTN_SERVICE) {
        out->p1 |= 1 << 14;
    }
}

static void idac_jvs_read_analogs(
        void *ctx,
        uint16_t *analogs,
        uint8_t nanalogs)
{
    struct idac_io_analog_state state;

    assert(analogs != NULL);
    assert(idac_dll.jvs_read_analogs != NULL);

    memset(&state, 0, sizeof(state));
    idac_dll.jvs_read_analogs(&state);

    if (nanalogs > 0) {
        analogs[0] = 0x8000 + state.wheel;
    }

    if (nanalogs > 1) {
        analogs[1] = state.accel;
    }

    if (nanalogs > 2) {
        analogs[2] = state.brake;
    }
}

static void idac_jvs_read_coin_counter(
        void *ctx,
        uint8_t slot_no,
        uint16_t *out)
{
    assert(idac_dll.jvs_read_coin_counter != NULL);

    if (slot_no > 0) {
        return;
    }

    idac_dll.jvs_read_coin_counter(out);
}
