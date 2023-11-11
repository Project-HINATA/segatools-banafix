#pragma once

#include <windows.h>

#include <stdbool.h>

struct deck_config {
    bool enable;
};

HRESULT deck_hook_init(const struct deck_config *cfg, int port);
