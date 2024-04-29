/* This is some sort of LCD display found on various cabinets. It is driven
   directly by amdaemon, and it has something to do with displaying the status
   of electronic payments.

   Part number in schematics is "VFD GP1232A02A FUTABA".

   Little else about this board is known. Black-holing the RS232 comms that it
   receives seems to be sufficient for the time being. */

#include <windows.h>

#include <assert.h>
#include <stdint.h>

#include "board/vfd.h"

#include "hook/iohook.h"

#include "hooklib/uart.h"

#include "util/dprintf.h"
#include "util/dump.h"

static HRESULT vfd_handle_irp(struct irp *irp);

static struct uart vfd_uart;
static uint8_t vfd_written[512];
static uint8_t vfd_readable[512];
UINT codepage;

HRESULT vfd_hook_init(const struct vfd_config *cfg, unsigned int port_no)
{
    assert(cfg != NULL);

    if (!cfg->enable) {
        return S_FALSE;
    }

    uart_init(&vfd_uart, port_no);
    vfd_uart.written.bytes = vfd_written;
    vfd_uart.written.nbytes = sizeof(vfd_written);
    vfd_uart.readable.bytes = vfd_readable;
    vfd_uart.readable.nbytes = sizeof(vfd_readable);

    codepage = GetACP();
    dprintf("VFD: hook enabled.\n");

    return iohook_push_handler(vfd_handle_irp);
}

static HRESULT vfd_handle_irp(struct irp *irp)
{
    HRESULT hr;

    assert(irp != NULL);

    if (!uart_match_irp(&vfd_uart, irp)) {
        return iohook_invoke_next(irp);
    }

    hr = uart_handle_irp(&vfd_uart, irp);

    if (FAILED(hr) || irp->op != IRP_OP_WRITE) {
        return hr;
    }

    uint8_t cmd = 0;
    uint8_t str_1[512];
    uint8_t str_2[512];
    uint8_t str_1_len = 0;
    uint8_t str_2_len = 0;
    for (size_t i = 0; i < vfd_uart.written.pos; i++) {
        if (vfd_uart.written.bytes[i] == 0x1B) {
            i++;
            cmd = vfd_uart.written.bytes[i];
            if (cmd == 0x30) {
                i += 3;
            }
            else if (cmd == 0x50) {
                i++;
            }
            continue;
        }
        if (cmd == 0x30) {
            str_1[str_1_len++] = vfd_uart.written.bytes[i];
        }
        else if (cmd == 0x50) {
            str_2[str_2_len++] = vfd_uart.written.bytes[i];
        }
    }

    if (str_1_len) {
        str_1[str_1_len++] = '\0';
        if (codepage != 932) {
            WCHAR buffer[512];
            MultiByteToWideChar(932, 0, (LPCSTR)str_1, str_1_len, buffer, str_1_len);
            char str_recode[str_1_len * 3];
            WideCharToMultiByte(codepage, 0, buffer, str_1_len, str_recode, str_1_len * 3, NULL, NULL);
            dprintf("VFD: %s\n", str_recode);
        }
        else {
            dprintf("VFD: %s\n", str_1);
        }
    }

    if (str_2_len) {
        str_2[str_2_len++] = '\0';
        if (codepage != 932) {
            WCHAR buffer[512];
            MultiByteToWideChar(932, 0, (LPCSTR)str_2, str_2_len, buffer, str_2_len);
            char str_recode[str_2_len * 3];
            WideCharToMultiByte(codepage, 0, buffer, str_2_len, str_recode, str_2_len * 3, NULL, NULL);
            dprintf("VFD: %s\n", str_recode);
        } else {
            dprintf("VFD: %s\n", str_2);
        }
    }

    // dprintf("VFD TX:\n");
    // dump_iobuf(&vfd_uart.written);
    vfd_uart.written.pos = 0;

    return hr;
}
