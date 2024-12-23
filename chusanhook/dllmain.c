/*
    "CHUNITHM NEW" (chusan) hook

    Devices

    USB:    837-15257-02 "Type 4" I/O Board
    COM1:   837-15330 Ground Slider

    [CVT mode (DIPSW2 ON)]

    COM2:   837-15093-06 LED Controller Board
    COM3:   837-15093-06 LED Controller Board
    COM4:   837-15286 "Gen 2" Aime Reader

    [SP mode (DIPSW2 OFF)]

    USB:    837-15067-02 USB Serial I/F Board
            connected to
            837-15093-06 LED Controller Board (COM20)
            837-15093-06 LED Controller Board (COM21)
    COM2:   200-6275 VFD GP1232A02A FUTABA Board
    COM4:   837-15396 "Gen 3" Aime Reader
*/

#include <windows.h>

#include <stddef.h>
#include <stdlib.h>

#include "amex/amex.h"

#include "board/sg-reader.h"
#include "board/vfd.h"

#include "chusanhook/config.h"
#include "chusanhook/io4.h"
#include "chusanhook/slider.h"

#include "chuniio/chuniio.h"

#include "hook/process.h"

#include "gfxhook/d3d9.h"
#include "gfxhook/gfx.h"

#include "hooklib/serial.h"
#include "hooklib/spike.h"

#include "platform/platform.h"

#include "util/dprintf.h"
#include "util/env.h"

static HMODULE chusan_hook_mod;
static process_entry_t chusan_startup;
static struct chusan_hook_config chusan_hook_cfg;

static DWORD CALLBACK chusan_pre_startup(void)
{
    HMODULE d3dc;
    HMODULE dbghelp;
    HRESULT hr;

    dprintf("--- Begin chusan_pre_startup ---\n");

    /* Pin the D3D shader compiler. This makes startup much faster. */

    d3dc = LoadLibraryW(L"D3DCompiler_43.dll");

    if (d3dc != NULL) {
        dprintf("Pinned shader compiler, hMod=%p\n", d3dc);
    } else {
        dprintf("Failed to load shader compiler!\n");
    }

    /* Pin dbghelp so the path hooks apply to it. */

    dbghelp = LoadLibraryW(L"dbghelp.dll");

    if (dbghelp != NULL) {
        dprintf("Pinned debug helper library, hMod=%p\n", dbghelp);
    } else {
        dprintf("Failed to load debug helper library!\n");
    }

    /* Config load */

    chusan_hook_config_load(&chusan_hook_cfg, get_config_path());

    /* Hook Win32 APIs */
    
    dvd_hook_init(&chusan_hook_cfg.dvd, chusan_hook_mod);
    gfx_hook_init(&chusan_hook_cfg.gfx);
    gfx_d3d9_hook_init(&chusan_hook_cfg.gfx, chusan_hook_mod);
    serial_hook_init();

    /* Initialize emulation hooks */

    hr = platform_hook_init(
        &chusan_hook_cfg.platform,
        "SDHD",
        "ACA2",
        chusan_hook_mod);

    if (FAILED(hr)) {
        goto fail;
    }

    hr = chuni_dll_init(&chusan_hook_cfg.dll, chusan_hook_mod);

    if (FAILED(hr)) {
        goto fail;
    }

    hr = chusan_io4_hook_init(&chusan_hook_cfg.io4);

    if (FAILED(hr)) {
        goto fail;
    }

    hr = slider_hook_init(&chusan_hook_cfg.slider);

    if (FAILED(hr)) {
        goto fail;
    }

    bool *dipsw = &chusan_hook_cfg.platform.system.dipsw[0];
    bool is_cvt = dipsw[2];

    for (int i = 0; i < 3; i++) {
        switch (i) {
        case 0:
            dprintf("DipSw: NetInstall: %s\n", dipsw[0] ? "Server" : "Client");
            break;

        case 1:
            dprintf("DipSw: Monitor Type: %dFPS\n", dipsw[1] ? 60 : 120);
            break;

        case 2:
            dprintf("DipSw: Cab Type: %s\n", is_cvt ? "CVT" : "SP");

            break;
        }
    }

    unsigned int first_port = is_cvt ? 2 : 20;

    if (!is_cvt) {
        hr = vfd_hook_init(&chusan_hook_cfg.vfd, 2);

        if (FAILED(hr)) {
            goto fail;
        }
    }

    if ( chuni_dll.led_init == NULL || chuni_dll.led_set_leds == NULL )
    {
        dprintf("IO DLL doesn't support led_init/led_set_leds, cannot start LED15093 hook\n");
    } else {
        hr = led15093_hook_init(&chusan_hook_cfg.led15093, 
            chuni_dll.led_init, chuni_dll.led_set_leds, first_port, 2, 2, 1);

        if (FAILED(hr)) {
            goto fail;
        }
    }

    hr = sg_reader_hook_init(&chusan_hook_cfg.aime, 4, is_cvt ? 2: 3, chusan_hook_mod);

    if (FAILED(hr)) {
        goto fail;
    }

    /* Initialize debug helpers */

    spike_hook_init(get_config_path());

    dprintf("---  End  chusan_pre_startup ---\n");

    /* Jump to EXE start address */

    return chusan_startup();

fail:
    ExitProcess(EXIT_FAILURE);
}

BOOL WINAPI DllMain(HMODULE mod, DWORD cause, void *ctx)
{
    HRESULT hr;

    if (cause != DLL_PROCESS_ATTACH) {
        return TRUE;
    }

    chusan_hook_mod = mod;

    hr = process_hijack_startup(chusan_pre_startup, &chusan_startup);

    if (!SUCCEEDED(hr)) {
        dprintf("Failed to hijack process startup: %x\n", (int) hr);
    }

    return SUCCEEDED(hr);
}
