#include <windows.h>

#include <assert.h>
#include <stddef.h>
#include <stdio.h>

#include "mu3io/config.h"


void mu3_io_config_load(
        struct mu3_io_config *cfg,
        const wchar_t *filename)
{
    assert(cfg != NULL);
    assert(filename != NULL);

    cfg->vk_test = GetPrivateProfileIntW(L"io4", L"test", VK_F1, filename);
    cfg->vk_service = GetPrivateProfileIntW(L"io4", L"service", VK_F2, filename);
    cfg->vk_coin = GetPrivateProfileIntW(L"io4", L"coin", VK_F3, filename);

    cfg->use_mouse = GetPrivateProfileIntW(L"io4", L"mouse", 0, filename);

    cfg->vk_left_1 = GetPrivateProfileIntW(L"io4", L"left1", 'A', filename);
    cfg->vk_left_2 = GetPrivateProfileIntW(L"io4", L"left2", 'S', filename);
    cfg->vk_left_3 = GetPrivateProfileIntW(L"io4", L"left3", 'D', filename);
    cfg->vk_left_side = GetPrivateProfileIntW(L"io4", L"leftSide", 'Q', filename);
    cfg->vk_right_side = GetPrivateProfileIntW(L"io4", L"rightSide", 'E', filename);
    cfg->vk_right_1 = GetPrivateProfileIntW(L"io4", L"right1", 'J', filename);
    cfg->vk_right_2 = GetPrivateProfileIntW(L"io4", L"right2", 'K', filename);
    cfg->vk_right_3 = GetPrivateProfileIntW(L"io4", L"right3", 'L', filename);
    cfg->vk_left_menu = GetPrivateProfileIntW(L"io4", L"leftMenu", 'U', filename);
    cfg->vk_right_menu = GetPrivateProfileIntW(L"io4", L"rightMenu", 'O', filename);
}
