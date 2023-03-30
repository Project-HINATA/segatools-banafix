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
static HRESULT controlbd_frame_decode(struct controlbd_req *dest, struct iobuf *iobuf);
static HRESULT controlbd_frame_dispatch(struct controlbd_req *dest);

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

    uart_init(&controlbd_uart, 11);
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
    struct controlbd_req req;
    HRESULT hr;

    assert(carol_dll.controlbd_init != NULL);

    if (irp->op == IRP_OP_OPEN) {
        dprintf("Control Board: Starting backend DLL\n");
        hr = carol_dll.controlbd_init();

        if (FAILED(hr)) {
            dprintf("Control Board: Backend DLL error: 0X%X\n", (int) hr);

            return hr;
        }
    }

    hr = uart_handle_irp(&controlbd_uart, irp);

    if (FAILED(hr) || irp->op != IRP_OP_WRITE) {
        return hr;
    }

    for (;;) {
#if 0
        dprintf("Control Board: TX Buffer:\n");
        dump_iobuf(&controlbd_uart.written);
#endif
        hr = controlbd_frame_decode(&req, &controlbd_uart.written);

        if (FAILED(hr)) {
            dprintf("Control Board: Deframe Error: 0X%X\n", (int) hr);

            return hr;
        }

        hr = controlbd_frame_dispatch(&req);
        if (FAILED(hr)) {
            dprintf("Control Board: Dispatch Error: 0X%X\n", (int) hr);

            return hr;
        }

        return hr;
    }
}

static HRESULT controlbd_frame_dispatch(struct controlbd_req *req)
{
    switch (req->cmd) {
    case CONTROLBD_CMD_UNK_10:
        return controlbd_req_noop(req->cmd);
    case CONTROLBD_CMD_UNK_7C:
        return controlbd_req_unk7c(req->cmd);
    case CONTROLBD_CMD_UNK_F0:
        return controlbd_req_unkF0(req->cmd);
    case CONTROLBD_CMD_UNK_30:
        return controlbd_req_noop(req->cmd);
    default:
        dprintf("Unhandled command 0x%02X\n", req->cmd);
        return controlbd_req_noop(req->cmd);
    }
}

static HRESULT controlbd_req_noop(uint8_t cmd)
{
    dprintf("Control Board: Noop cmd 0x%02X\n", cmd);
    uint8_t resp[] = { 0xE0, 0x01, 0x11, 0x03, 0x01, 0x00, 0x01, 0x17 };
    resp[5] = cmd;
    resp[7] = 0x17 + cmd;
    iobuf_write(&controlbd_uart.readable, resp, 8);

    return S_OK;
}

static HRESULT controlbd_req_unk7c(uint8_t cmd)
{
    dprintf("Control Board: Cmd 0x7C\n");
    uint8_t resp[] = { 0xE0, 0x01, 0x11, 0x04, 0x01, 0x7C, 0x01, 0x07, 0x9B };
    iobuf_write(&controlbd_uart.readable, resp, 9);

    return S_OK;
}

static HRESULT controlbd_req_unkF0(uint8_t cmd)
{
    dprintf("Control Board: Cmd 0xF0\n");
    uint8_t resp[] = { 0xE0, 0x01, 0x11, 0x0A, 0x01, 0xF0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E };
    iobuf_write(&controlbd_uart.readable, resp, 16);

    return S_OK;
}

/* Decodes the response into a struct that's easier to work with. TODO: Further validation */
static HRESULT controlbd_frame_decode(struct controlbd_req *dest, struct iobuf *iobuf)
{
    int initial_pos = iobuf->pos;
    int processed_pos = 0;
    uint8_t check = 0; 
    bool escape = false;   
    dump_iobuf(iobuf);

    dest->sync = iobuf->bytes[0];

    dest->dest = iobuf->bytes[1];
    check += dest->dest;

    dest->src = iobuf->bytes[2];
    check += dest->src;

    dest->data_len = iobuf->bytes[3];
    check += dest->data_len;

    dest->cmd = iobuf->bytes[4];
    check += dest->cmd;

    if (dest->data_len > 1) {
        for (int i = 0; i < iobuf->pos - 6; i++) {
            if (iobuf->bytes[i+6] == 0xD0) {
                escape = true;
            }
            
            dest->data[i] = iobuf->bytes[i+6];
            
            if (escape) {                
                dest->data[i]++;
                escape = false;
            }
            
            check += dest->data[i];
        }
        
    }
    dest->checksum = iobuf->bytes[iobuf->pos - 1];
    if (escape) {
        dest->checksum++;
        escape = false;
    }

    iobuf->pos = 0;

    if (dest->sync != 0xe0) {
        dprintf("Control Board: Sync error, expected 0xe0, got 0X%02X\n", dest->sync);
        return E_FAIL;
    }
    
    if (dest->checksum != (check & 0xFF)) {
        dprintf("Control Board: Checksum error, expected 0X%02X, got 0X%02X\n", dest->checksum, check);
        return E_FAIL;
    }

    return S_OK;
}