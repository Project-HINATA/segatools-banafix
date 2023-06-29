#include <assert.h>
#include <stddef.h>

#include "board/config.h"
#include "board/sg-reader.h"

#include "hooklib/config.h"
#include "hooklib/dvd.h"

#include "idachook/config.h"
#include "idachook/idac-dll.h"

#include "platform/config.h"
#include "platform/platform.h"

void idac_dll_config_load(
        struct idac_dll_config *cfg,
        const wchar_t *filename)
{
    assert(cfg != NULL);
    assert(filename != NULL);

    GetPrivateProfileStringW(
            L"idacio",
            L"path",
            L"",
            cfg->path,
            _countof(cfg->path),
            filename);
}

void idac_hook_config_load(
        struct idac_hook_config *cfg,
        const wchar_t *filename)
{
    assert(cfg != NULL);
    assert(filename != NULL);

    platform_config_load(&cfg->platform, filename);
    aime_config_load(&cfg->aime, filename);
    idac_dll_config_load(&cfg->dll, filename);
    zinput_config_load(&cfg->zinput, filename);
    dvd_config_load(&cfg->dvd, filename);
    io4_config_load(&cfg->io4, filename);
}

void zinput_config_load(struct zinput_config *cfg, const wchar_t *filename)
{
    assert(cfg != NULL);
    assert(filename != NULL);

    cfg->enable = GetPrivateProfileIntW(L"zinput", L"enable", 1, filename);
}
