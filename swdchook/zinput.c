#include <windows.h>
#include <xinput.h>

#include <assert.h>
#include <stdlib.h>

#include "swdchook/config.h"
#include "swdchook/zinput.h"

#include "hook/table.h"

#include "util/dprintf.h"

DWORD WINAPI hook_XInputGetState(DWORD dwUserIndex, XINPUT_STATE *pState);

static const struct hook_symbol zinput_hook_syms[] = {
    {
        .name   = "XInputGetState",
        .patch  = hook_XInputGetState,
        .link   = NULL
    },
};

static struct zinput_config zinput_config;
static bool zinput_hook_initted;

void zinput_hook_init(struct zinput_config *cfg, HINSTANCE self)
{
    assert(cfg != NULL);

    if (!cfg->enable) {
        return;
    }

    if (zinput_hook_initted) {
        return;
    }

    memcpy(&zinput_config, cfg, sizeof(*cfg));
    hook_table_apply(
            NULL,
            "XINPUT9_1_0.dll",
            zinput_hook_syms,
            _countof(zinput_hook_syms));

    hook_table_apply(
            NULL,
            "XINPUT1_1.dll",
            zinput_hook_syms,
            _countof(zinput_hook_syms));

    hook_table_apply(
            NULL,
            "XINPUT1_2.dll",
            zinput_hook_syms,
            _countof(zinput_hook_syms));

    hook_table_apply(
            NULL,
            "XINPUT1_3.dll",
            zinput_hook_syms,
            _countof(zinput_hook_syms));

    hook_table_apply(
            NULL,
            "XINPUT1_4.dll",
            zinput_hook_syms,
            _countof(zinput_hook_syms));

    zinput_hook_initted = true;

    dprintf("ZInput: Blocking built-in XInput support\n");
}

DWORD WINAPI hook_XInputGetState(DWORD dwUserIndex, XINPUT_STATE *pState)
{
    // dprintf("ZInput: XInputGetState hook hit\n");

    return ERROR_SUCCESS;
}
