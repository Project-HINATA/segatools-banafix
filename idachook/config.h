#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "amex/amex.h"

#include "board/sg-reader.h"

#include "gfxhook/gfx.h"

#include "hooklib/dvd.h"

#include "idachook/idac-dll.h"
#include "idachook/zinput.h"

#include "platform/platform.h"

struct idac_hook_config {
    struct platform_config platform;
    struct amex_config amex;
    struct aime_config aime;
    struct dvd_config dvd;
    struct idac_dll_config dll;
    struct zinput_config zinput;
};

void idac_dll_config_load(
        struct idac_dll_config *cfg,
        const wchar_t *filename);

void idac_hook_config_load(
        struct idac_hook_config *cfg,
        const wchar_t *filename);

void zinput_config_load(struct zinput_config *cfg, const wchar_t *filename);
