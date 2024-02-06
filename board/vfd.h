#pragma once

#include <windows.h>

struct vfd_config {
    bool enable;
};


HRESULT vfd_hook_init(const struct vfd_config *cfg, unsigned int port_no);
