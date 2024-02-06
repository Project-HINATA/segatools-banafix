#include <windows.h>

#include <assert.h>
#include <stddef.h>
#include <stdio.h>

#include "fgoio/config.h"


void fgo_io_config_load(
        struct fgo_io_config *cfg,
        const wchar_t *filename)
{
    assert(cfg != NULL);
    assert(filename != NULL);

    cfg->vk_test = GetPrivateProfileIntW(L"io4", L"test", VK_F1, filename);
    cfg->vk_service = GetPrivateProfileIntW(L"io4", L"service", VK_F2, filename);
    cfg->vk_coin = GetPrivateProfileIntW(L"io4", L"coin", VK_F3, filename);
}
