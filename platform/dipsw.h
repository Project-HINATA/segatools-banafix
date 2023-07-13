#pragma once

#include <windows.h>

#include <stdbool.h>
#include <stdint.h>

#include "platform/vfs.h"

struct dipsw_config {
    bool enable;
    bool dipsw[8];
};

HRESULT dipsw_init(const struct dipsw_config *cfg, const struct vfs_config *vfs_cfg);
