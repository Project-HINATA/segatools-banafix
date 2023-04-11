#include <windows.h>

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "hook/iobuf.h"
#include "hook/iohook.h"

#include "carolhook/carol-dll.h"
#include "carolhook/ledbd.h"

#include "hooklib/uart.h"

#include "util/dprintf.h"
#include "util/dump.h"

static HRESULT ledbd_handle_irp(struct irp *irp);
static HRESULT ledbd_handle_irp_locked(struct irp *irp);
static HRESULT ledbd_frame_decode(struct ledbd_req *dest, struct iobuf *iobuf);
static HRESULT ledbd_frame_dispatch(struct ledbd_req *dest);

static HRESULT ledbd_req_noop(uint8_t cmd);
static HRESULT ledbd_req_unk7c(uint8_t cmd);
static HRESULT ledbd_req_unkF0(uint8_t cmd);

static CRITICAL_SECTION ledbd_lock;
static struct uart ledbd_uart;
static uint8_t ledbd_written_bytes[520];
static uint8_t ledbd_readable_bytes[520];

HRESULT ledbd_hook_init(const struct ledbd_config *cfg)
{
    if (!cfg->enable) {
        return S_OK;
    }

    InitializeCriticalSection(&ledbd_lock);

    uart_init(&ledbd_uart, 11);
    ledbd_uart.written.bytes = ledbd_written_bytes;
    ledbd_uart.written.nbytes = sizeof(ledbd_written_bytes);
    ledbd_uart.readable.bytes = ledbd_readable_bytes;
    ledbd_uart.readable.nbytes = sizeof(ledbd_readable_bytes);

    dprintf("LED Board: Init\n");

    return iohook_push_handler(ledbd_handle_irp);
}

static HRESULT ledbd_handle_irp(struct irp *irp)
{
    HRESULT hr;

    assert(irp != NULL);

    if (!uart_match_irp(&ledbd_uart, irp)) {
        return iohook_invoke_next(irp);
    }

    EnterCriticalSection(&ledbd_lock);
    hr = ledbd_handle_irp_locked(irp);
    LeaveCriticalSection(&ledbd_lock);

    return hr;
}

static HRESULT ledbd_handle_irp_locked(struct irp *irp)
{
    struct ledbd_req req;
    HRESULT hr;

    assert(carol_dll.ledbd_init != NULL);

    if (irp->op == IRP_OP_OPEN) {
        dprintf("LED Board: Starting backend DLL\n");
        hr = carol_dll.ledbd_init();

        if (FAILED(hr)) {
            dprintf("LED Board: Backend DLL error: 0X%X\n", (int) hr);

            return hr;
        }
    }

    hr = uart_handle_irp(&ledbd_uart, irp);

    if (FAILED(hr) || irp->op != IRP_OP_WRITE) {
        return hr;
    }

    for (;;) {
#if 0
        dprintf("LED Board: TX Buffer:\n");
        dump_iobuf(&ledbd_uart.written);
#endif
        hr = ledbd_frame_decode(&req, &ledbd_uart.written);

        if (FAILED(hr)) {
            dprintf("LED Board: Deframe Error: 0X%X\n", (int) hr);

            return hr;
        }

        hr = ledbd_frame_dispatch(&req);
        if (FAILED(hr)) {
            dprintf("LED Board: Dispatch Error: 0X%X\n", (int) hr);

            return hr;
        }

        return hr;
    }
}

static HRESULT ledbd_frame_dispatch(struct ledbd_req *req)
{
    switch (req->cmd) {
    case LEDBD_CMD_UNK_10:
        return ledbd_req_noop(req->cmd);
    case LEDBD_CMD_UNK_7C:
        return ledbd_req_unk7c(req->cmd);
    case LEDBD_CMD_UNK_F0:
        return ledbd_req_unkF0(req->cmd);
    case LEDBD_CMD_UNK_30:
        return ledbd_req_noop(req->cmd);
    default:
        dprintf("Unhandled command 0x%02X\n", req->cmd);
        return ledbd_req_noop(req->cmd);
    }
}

static HRESULT ledbd_req_noop(uint8_t cmd)
{
    dprintf("LED Board: Noop cmd 0x%02X\n", cmd);
    uint8_t resp[] = { 0xE0, 0x01, 0x11, 0x03, 0x01, 0x00, 0x01, 0x17 };
    resp[5] = cmd;
    resp[7] = 0x17 + cmd;
    iobuf_write(&ledbd_uart.readable, resp, 8);

    return S_OK;
}

static HRESULT ledbd_req_unk7c(uint8_t cmd)
{
    dprintf("LED Board: Cmd 0x7C\n");
    uint8_t resp[] = { 0xE0, 0x01, 0x11, 0x04, 0x01, 0x7C, 0x01, 0x07, 0x9B };
    iobuf_write(&ledbd_uart.readable, resp, 9);

    return S_OK;
}

static HRESULT ledbd_req_unkF0(uint8_t cmd)
{
    dprintf("LED Board: Cmd 0xF0\n");
    uint8_t resp[] = { 0xE0, 0x01, 0x11, 0x0A, 0x01, 0xF0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E };
    iobuf_write(&ledbd_uart.readable, resp, 16);

    return S_OK;
}

/* Decodes the response into a struct that's easier to work with. TODO: Further validation */
static HRESULT ledbd_frame_decode(struct ledbd_req *dest, struct iobuf *iobuf)
{
    int initial_pos = iobuf->pos;
    int processed_pos = 0;
    uint8_t check = 0; 
    bool escape = false;

    dest->sync = iobuf->bytes[0];

    dest->dest = iobuf->bytes[1];
    check += dest->dest;

    dest->src = iobuf->bytes[2];
    check += dest->src;

    dest->data_len = iobuf->bytes[3];
    check += dest->data_len;

    dest->cmd = iobuf->bytes[4];
    check += dest->cmd;

    for (int i = 0; i < dest->data_len - 1; i++) {
        if (iobuf->bytes[i+5] == 0xD0) {
            escape = true;
            check += 0xD0;
            continue;
        }
        
        dest->data[i] = iobuf->bytes[i+5];
        
        if (escape) {                
            dest->data[i]++;
            escape = false;
        }
        
        check += iobuf->bytes[i+5];
    }

    dest->checksum = iobuf->bytes[iobuf->pos - 1];
    if (escape) {
        dest->checksum++;
        escape = false;
    }

    iobuf->pos = 0;

    if (dest->sync != 0xe0) {
        dprintf("LED Board: Sync error, expected 0xe0, got 0X%02X\n", dest->sync);
        dump_iobuf(iobuf);
        return E_FAIL;
    }
    
    if (dest->checksum != (check & 0xFF)) {
        dprintf("LED Board: Checksum error, expected 0X%02X, got 0X%02X\n", dest->checksum, check);
        dump_iobuf(iobuf);
        return E_FAIL;
    }

    return S_OK;
}