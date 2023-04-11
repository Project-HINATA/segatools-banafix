#include <windows.h>

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "hook/iobuf.h"
#include "hook/iohook.h"

#include "carolhook/carol-dll.h"
#include "carolhook/controlbd.h"

#include "hooklib/uart.h"

#include "util/dprintf.h"
#include "util/dump.h"

static HRESULT controlbd_handle_irp(struct irp *irp);
static HRESULT controlbd_handle_irp_locked(struct irp *irp);
//static HRESULT controlbd_frame_decode(struct controlbd_req *dest, struct iobuf *iobuf);
//static HRESULT controlbd_frame_dispatch(struct controlbd_req *dest);

static HRESULT controlbd_req_noop(uint8_t cmd);
static HRESULT controlbd_req_unk7c(uint8_t cmd);
static HRESULT controlbd_req_unkF0(uint8_t cmd);

static CRITICAL_SECTION controlbd_lock;
static struct uart controlbd_uart;
static uint8_t controlbd_written_bytes[520];
static uint8_t controlbd_readable_bytes[520];

HRESULT controlbd_hook_init(const struct controlbd_config *cfg)
{
    if (!cfg->enable) {
        return S_OK;
    }

    InitializeCriticalSection(&controlbd_lock);

    uart_init(&controlbd_uart, 12);
    controlbd_uart.written.bytes = controlbd_written_bytes;
    controlbd_uart.written.nbytes = sizeof(controlbd_written_bytes);
    controlbd_uart.readable.bytes = controlbd_readable_bytes;
    controlbd_uart.readable.nbytes = sizeof(controlbd_readable_bytes);

    dprintf("Control Board: Init\n");

    return iohook_push_handler(controlbd_handle_irp);
}


static HRESULT controlbd_handle_irp(struct irp *irp)
{
    HRESULT hr;

    assert(irp != NULL);

    if (!uart_match_irp(&controlbd_uart, irp)) {
        return iohook_invoke_next(irp);
    }

    EnterCriticalSection(&controlbd_lock);
    hr = controlbd_handle_irp_locked(irp);
    LeaveCriticalSection(&controlbd_lock);

    return hr;
}

static HRESULT controlbd_handle_irp_locked(struct irp *irp)
{
    HRESULT hr;

    assert(carol_dll.controlbd_init != NULL);

    if (irp->op == IRP_OP_OPEN) {
        dprintf("LED Board: Starting backend DLL\n");
        hr = carol_dll.controlbd_init();

        if (FAILED(hr)) {
            dprintf("LED Board: Backend DLL error: 0X%X\n", (int) hr);

            return hr;
        }
    }

    hr = uart_handle_irp(&controlbd_uart, irp);

    if (FAILED(hr) || irp->op != IRP_OP_WRITE) {
        return hr;
    }

    for (;;) {
#if 1
        dprintf("LED Board: TX Buffer:\n");
        dump_iobuf(&controlbd_uart.written);
#endif
        //hr = controlbd_frame_decode(&req, &controlbd_uart.written);

        if (FAILED(hr)) {
            dprintf("LED Board: Deframe Error: 0X%X\n", (int) hr);

            return hr;
        }

        //hr = controlbd_frame_dispatch(&req);
        if (FAILED(hr)) {
            dprintf("LED Board: Dispatch Error: 0X%X\n", (int) hr);

            return hr;
        }

        controlbd_uart.written.pos = 0;
        return hr;
    }
}