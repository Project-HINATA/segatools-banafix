#include <windows.h>

#include <assert.h>
#include <stdlib.h>

#include "idachook/idac-dll.h"

#include "util/dll-bind.h"
#include "util/dprintf.h"

const struct dll_bind_sym idac_dll_syms[] = {
    {
        .sym = "idac_io_init",
        .off = offsetof(struct idac_dll, init),
    }, {
        .sym = "idac_io_poll",
        .off = offsetof(struct idac_dll, poll),
    }, {
        .sym = "idac_io_get_opbtns",
        .off = offsetof(struct idac_dll, get_opbtns),
    }, {
        .sym = "idac_io_get_gamebtns",
        .off = offsetof(struct idac_dll, get_gamebtns),
    }, {
        .sym = "idac_io_get_shifter",
        .off = offsetof(struct idac_dll, get_shifter),
    }, {
        .sym = "idac_io_get_analogs",
        .off = offsetof(struct idac_dll, get_analogs),
    }
};

struct idac_dll idac_dll;

// Copypasta DLL binding and diagnostic message boilerplate.
// Not much of this lends itself to being easily factored out. Also there
// will be a lot of API-specific branching code here eventually as new API
// versions get defined, so even though these functions all look the same
// now this won't remain the case forever.

HRESULT idac_dll_init(const struct idac_dll_config *cfg, HINSTANCE self)
{
    uint16_t (*get_api_version)(void);
    const struct dll_bind_sym *sym;
    HINSTANCE owned;
    HINSTANCE src;
    HRESULT hr;

    assert(cfg != NULL);
    assert(self != NULL);

    if (cfg->path[0] != L'\0') {
        owned = LoadLibraryW(cfg->path);

        if (owned == NULL) {
            hr = HRESULT_FROM_WIN32(GetLastError());
            dprintf("IDAC IO: Failed to load IO DLL: %lx: %S\n",
                    hr,
                    cfg->path);

            goto end;
        }

        dprintf("IDAC IO: Using custom IO DLL: %S\n", cfg->path);
        src = owned;
    } else {
        owned = NULL;
        src = self;
    }

    get_api_version = (void *) GetProcAddress(src, "idac_io_get_api_version");

    if (get_api_version != NULL) {
        idac_dll.api_version = get_api_version();
    } else {
        idac_dll.api_version = 0x0100;
        dprintf("Custom IO DLL does not expose idac_io_get_api_version, "
                "assuming API version 1.0.\n"
                "Please ask the developer to update their DLL.\n");
    }

    if (idac_dll.api_version >= 0x0200) {
        hr = E_NOTIMPL;
        dprintf("IDAC IO: Custom IO DLL implements an unsupported "
                "API version (%#04x). Please update Segatools.\n",
                idac_dll.api_version);

        goto end;
    }

    sym = idac_dll_syms;
    hr = dll_bind(&idac_dll, src, &sym, _countof(idac_dll_syms));

    if (FAILED(hr)) {
        if (src != self) {
            dprintf("IDAC IO: Custom IO DLL does not provide function "
                    "\"%s\". Please contact your IO DLL's developer for "
                    "further assistance.\n",
                    sym->sym);

            goto end;
        } else {
            dprintf("Internal error: could not reflect \"%s\"\n", sym->sym);
        }
    }

    owned = NULL;

end:
    if (owned != NULL) {
        FreeLibrary(owned);
    }

    return hr;
}
