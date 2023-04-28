#include <windows.h>

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "carolhook/carol-dll.h"
#include "carolhook/touch.h"

#include "hooklib/uart.h"

#include "util/dprintf.h"
#include "util/dump.h"

/**
 * CMDS for M3 EX series
 * CX -> Calibrate Extend, preform callibration
 * MS -> Mode Stream, enters stream mode
 * R  -> Reset, resets the device
 * RD -> Reset Default, Resets the device to factory
 * Z  -> Null, keepalive command
 * NM -> Name, return the name of the device (not documented?)
 * OI -> Output Identity, output unique device identity, SC followed by 4 characters
 * UT -> Unit Type, returns controller unit type + status
 */

static HRESULT touch_handle_irp(struct irp *irp);
static HRESULT touch_handle_irp_locked(struct irp *irp);
static HRESULT touch_frame_decode(struct touch_req *dest, struct iobuf *iobuf);

static HRESULT handle_touch_ack_cmd(const struct touch_req *req);
static HRESULT handle_touch_name_cmd(const struct touch_req *req);
static HRESULT handle_touch_id_cmd(const struct touch_req *req);

static CRITICAL_SECTION touch_lock;
static struct uart touch_uart;
static uint8_t touch_written_bytes[520];
static uint8_t touch_readable_bytes[520];
static bool should_stream = false;

HRESULT touch_hook_init(const struct touch_config *cfg)
{
    if (!cfg->enable) {
        return S_OK;
    }
    
    InitializeCriticalSection(&touch_lock);

    uart_init(&touch_uart, 1);
    touch_uart.written.bytes = touch_written_bytes;
    touch_uart.written.nbytes = sizeof(touch_written_bytes);
    touch_uart.readable.bytes = touch_readable_bytes;
    touch_uart.readable.nbytes = sizeof(touch_readable_bytes);

    dprintf("Touchscreen: Init\n");

    return iohook_push_handler(touch_handle_irp);
    return S_OK;
}

static HRESULT touch_handle_irp(struct irp *irp)
{
    HRESULT hr;

    assert(irp != NULL);

    if (!uart_match_irp(&touch_uart, irp)) {
        return iohook_invoke_next(irp);
    }

    EnterCriticalSection(&touch_lock);
    hr = touch_handle_irp_locked(irp);
    LeaveCriticalSection(&touch_lock);

    return hr;
}

static HRESULT touch_handle_irp_locked(struct irp *irp)
{
    struct touch_req req;
    HRESULT hr;

    assert(carol_dll.touch_init != NULL);

    if (irp->op == IRP_OP_OPEN) {
        dprintf("Touchscreen: Starting backend DLL\n");
        hr = carol_dll.touch_init();

        if (FAILED(hr)) {
            dprintf("Touchscreen: Backend DLL error: %X\n", (int) hr);

            return hr;
        }
    }

    hr = uart_handle_irp(&touch_uart, irp);

    if (FAILED(hr) || irp->op != IRP_OP_WRITE) {
        return hr;
    }

    for (;;) {
#if 0
        dprintf("Touchscreen: TX Buffer:\n");
        dump_iobuf(&touch_uart.written);
#endif
        hr = touch_frame_decode(&req, &touch_uart.written);

        if (FAILED(hr)) {
            dprintf("Touchscreen: Deframe Error: %X\n", (int) hr);

            return hr;
        }

        if (!strcmp("Z", (char *)req.cmd)) {
            hr = handle_touch_ack_cmd(&req);
        }
        else if (!strcmp("OI", (char *)req.cmd)) {
            hr = handle_touch_id_cmd(&req);
            should_stream = true; // possibly send stuff after we get this?
        }
        else if (!strcmp("NM", (char *)req.cmd)) {
            hr = handle_touch_name_cmd(&req);
        }
        else if (!strcmp("R", (char *)req.cmd)) {
            should_stream = false;
            dprintf("Touch: Reset\n");
            hr = handle_touch_ack_cmd(&req);
        }
        else {
            dprintf("Touchscreen: Unhandled cmd %s\n", (char *)req.cmd);
            hr = handle_touch_ack_cmd(&req);
        }

        return hr;
    }
}

static HRESULT handle_touch_ack_cmd(const struct touch_req *req)
{
    return iobuf_write(&touch_uart.readable, "\0010\015", 3);
}

static HRESULT handle_touch_name_cmd(const struct touch_req *req)
{
    dprintf("Touch: Get Name\n");
    return iobuf_write(&touch_uart.readable, "\001EX1234 EX1234\015", 15);
}

static HRESULT handle_touch_id_cmd(const struct touch_req *req)
{
    dprintf("Touch: Get ID\n");
    return iobuf_write(&touch_uart.readable, "\001EX1234\015", 8);
}

/* Decodes the response into a struct that's easier to work with. */
static HRESULT touch_frame_decode(struct touch_req *dest, struct iobuf *iobuf)
{
    dest->sync = iobuf->bytes[0];
    memset(dest->cmd, 0, sizeof(dest->cmd));
    size_t data_len = 0;

    for (int i = 1; i < 255; i++) {
        if (iobuf->bytes[i] == 0x0D) { break; }
        dest->cmd[i - 1] = iobuf->bytes[i];
        data_len++;
    }

    dest->tail = iobuf->bytes[data_len + 1];
    dest->data_len = data_len;

    iobuf->pos = 0;

    if (dest->sync != 1 || dest->tail != 0x0D) {
        dprintf("Touch: Data recieve error, sync: 0x%02X (expected 0x01) tail: 0x%02X (expected 0x0D)", dest->sync, dest->tail);
        return E_FAIL;
    }

    return S_OK;
}