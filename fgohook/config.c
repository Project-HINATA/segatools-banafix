#include <assert.h>
#include <stddef.h>

#include "board/config.h"

#include "hooklib/config.h"
#include "hooklib/dvd.h"

#include "fgohook/config.h"

#include "platform/config.h"

void fgo_dll_config_load(
        struct fgo_dll_config *cfg,
        const wchar_t *filename)
{
    assert(cfg != NULL);
    assert(filename != NULL);

    GetPrivateProfileStringW(
            L"fgoio",
            L"path",
            L"",
            cfg->path,
            _countof(cfg->path),
            filename);
}

void ftdi_config_load(struct ftdi_config *cfg, const wchar_t *filename)
{
    assert(cfg != NULL);
    assert(filename != NULL);

    cfg->enable = GetPrivateProfileIntW(L"ftdi", L"enable", 1, filename);
}

void led1509306_config_load(struct led1509306_config *cfg, const wchar_t *filename)
{
    assert(cfg != NULL);
    assert(filename != NULL);

    wchar_t tmpstr[16];

    memset(cfg->board_number, ' ', sizeof(cfg->board_number));
    memset(cfg->chip_number, ' ', sizeof(cfg->chip_number));
    
    cfg->enable = GetPrivateProfileIntW(L"ledstrip", L"enable", 1, filename);
    cfg->port_no = GetPrivateProfileIntW(L"ledstrip", L"port", 21, filename);
    cfg->fw_ver = GetPrivateProfileIntW(L"ledstrip", L"fw_ver", 0xA0, filename);
    cfg->fw_sum = GetPrivateProfileIntW(L"ledstrip", L"fw_sum", 0xaa53, filename);
    
    GetPrivateProfileStringW(L"ledstrip", L"board_number", L"15093-06", tmpstr, _countof(tmpstr), filename);
    size_t n = wcstombs(cfg->board_number, tmpstr, sizeof(cfg->board_number));
    for (int i = n; i < sizeof(cfg->board_number); i++)
    {
        cfg->board_number[i] = ' ';
    }
    
    GetPrivateProfileStringW(L"ledstrip", L"chip_number", L"6710A", tmpstr, _countof(tmpstr), filename);
    n = wcstombs(cfg->chip_number, tmpstr, sizeof(cfg->chip_number));
    for (int i = n; i < sizeof(cfg->chip_number); i++)
    {
        cfg->chip_number[i] = ' ';
    } 
}

void fgo_deck_config_load(
        struct deck_config *cfg,
        const wchar_t *filename)
{
    assert(cfg != NULL);
    assert(filename != NULL);

    cfg->enable = GetPrivateProfileIntW(L"deck", L"enable", 1, filename);
}

void fgo_hook_config_load(
        struct fgo_hook_config *cfg,
        const wchar_t *filename)
{
    assert(cfg != NULL);
    assert(filename != NULL);

    platform_config_load(&cfg->platform, filename);
    aime_config_load(&cfg->aime, filename);
    dvd_config_load(&cfg->dvd, filename);
    io4_config_load(&cfg->io4, filename);
    touch_screen_config_load(&cfg->touch, filename);
    printer_config_load(&cfg->printer, filename);
    fgo_deck_config_load(&cfg->deck, filename);
    ftdi_config_load(&cfg->ftdi, filename);
    led1509306_config_load(&cfg->led1509306, filename);
    fgo_dll_config_load(&cfg->dll, filename);
}
