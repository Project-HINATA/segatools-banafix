#pragma once

#include <windows.h>

#include <stdbool.h>
#include <stdint.h>

#include "platform/vfs.h"

struct system_config {
    bool enable;
    bool freeplay;
    bool dipsw[8];
};

HRESULT system_init(const struct system_config *cfg, const struct vfs_config *vfs_cfg);
