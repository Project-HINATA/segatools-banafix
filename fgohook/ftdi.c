/*
    SEGA 837-14509-02 USB -> Serial Adapter hook

    The 837-15093-06 LED controller is connected to the ALLS with an adapter.
    This tiny board has a FTDI FT232BL chip, and is referenced in schematics as
    "USB-SER I/F BD".

    The game queries the presence of the FTDI board itself, followed by a
    registry check to see which port number is assigned to the FTDI board.
    If these fail, the "CABINET LED" check on startup will always return "NG".
*/

#include <windows.h>

#include <assert.h>

#include "fgohook/ftdi.h"

#include "hook/iohook.h"

#include "hooklib/setupapi.h"

#include "util/dprintf.h"

static struct ftdi_config ftdi_cfg;

static HANDLE ftdi_fd;

HRESULT ftdi_hook_init(const struct ftdi_config *cfg)
{
    HRESULT hr;

    assert(cfg != NULL);

    if (!cfg->enable) {
        return S_FALSE;
    }

    memcpy(&ftdi_cfg, cfg, sizeof(*cfg));

    hr = iohook_open_nul_fd(&ftdi_fd);

    if (FAILED(hr)) {
        return hr;
    }

    hr = setupapi_add_phantom_dev(&ftdi_guid, L"$ftdi");

    if (FAILED(hr)) {
        return hr;
    }

    dprintf("FTDI: Hook enabled.\n");
    return S_OK;
}
