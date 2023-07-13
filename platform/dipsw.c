#include <windows.h>
#include <ntstatus.h>

#include <assert.h>
#include <string.h>
// #include <zlib.h>

#include "platform/dipsw.h"
#include "platform/vfs.h"

#include "util/dprintf.h"
#include "util/str.h"
#include "util/crc.h"

#define DATA_SIZE 503
#define BLOCK_SIZE (sizeof(uint32_t) + 4 + 1 + DATA_SIZE)

#pragma pack(push, 1)

typedef struct
{
    uint32_t checksum;
    char padding_1[4];
    uint8_t dip_switches;
    char data[DATA_SIZE];
} DipSwitchBlock;

typedef struct
{
    DipSwitchBlock dip_switch_block;
    char *data;
} DipSwitches;

#pragma pack(pop)

static DipSwitches dip_switches;

static struct dipsw_config dipsw_config;
static struct vfs_config vfs_config;

static void dipsw_read_sysfile(const wchar_t *sys_file);
static void dipsw_save_sysfile(const wchar_t *sys_file);

HRESULT dipsw_init(const struct dipsw_config *cfg, const struct vfs_config *vfs_cfg)
{
    HRESULT hr;
    wchar_t sys_file_path[MAX_PATH];

    assert(cfg != NULL);
    assert(vfs_cfg != NULL);

    if (!cfg->enable)
    {
        return S_FALSE;
    }

    memcpy(&dipsw_config, cfg, sizeof(*cfg));

    sys_file_path[0] = L'\0';
    // concatenate vfs_config.amfs with L"sysfile.dat"
    wcsncpy(sys_file_path, vfs_cfg->amfs, MAX_PATH);
    wcsncat(sys_file_path, L"\\sysfile.dat", MAX_PATH);

    dipsw_read_sysfile(sys_file_path);

    // now write the dipsw_config.dipsw to the dip_switch_block
    dipsw_save_sysfile(sys_file_path);

    return S_OK;
}

static void dipsw_read_sysfile(const wchar_t *sys_file)
{
    FILE *f = _wfopen(sys_file, L"r");

    if (f == NULL)
    {
        dprintf("First run detected, DipSw settings can only be applied AFTER the first run\n");
        return;
    }

    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (file_size != 0x6000)
    {
        dprintf("Invalid sysfile.dat file size\n");
        fclose(f);

        return;
    }

    dip_switches.data = malloc(file_size);
    fread(dip_switches.data, 1, file_size, f);
    fclose(f);

    // memcpy(dip_switches.dip_switch_block, dip_switches.data + 0x2800, BLOCK_SIZE);
    memcpy(&dip_switches.dip_switch_block, dip_switches.data + 0x2800, BLOCK_SIZE);
}

static void dipsw_save_sysfile(const wchar_t *sys_file)
{
    uint8_t dipsw = 0;
    // open the sysfile.dat for writing in bytes mode
    FILE *f = _wfopen(sys_file, L"rb+");

    if (f == NULL)
    {
        return;
    }

    // write the dipsw_config.dipsw to the dip_switch_block
    for (int i = 0; i < 8; i++)
    {
        if (dipsw_config.dipsw[i])
        {
            // print which dipsw is enabled
            dprintf("DipSw: DipSw%d=1 set\n", i + 1);
            dipsw |= (1 << i);
        }
    }

    dip_switches.dip_switch_block.dip_switches = dipsw;

    // calculate the new checksum, skip the old crc32 value
    // which is at the beginning of the block, thats's why the +4
    // conver the struct to chars in order for the crc32 calculation to work
    dip_switches.dip_switch_block.checksum = crc32(
        (char *)&dip_switches.dip_switch_block + 4, BLOCK_SIZE - 4, 0);

    // build the new dip switch block
    char block[BLOCK_SIZE];
    memcpy(block, (char *)&dip_switches.dip_switch_block, BLOCK_SIZE);

    // replace the old block with the new one
    memcpy(dip_switches.data + 0x2800, block, BLOCK_SIZE);
    memcpy(dip_switches.data + 0x5800, block, BLOCK_SIZE);

    // print the dip_switch_block in hex
    /*
    dprintf("DipSw Block: ");
    for (size_t i = 0; i < BLOCK_SIZE; i++)
    {
        dprintf("%02X ", ((uint8_t *)&dip_switches.dip_switch_block)[i]);
    }
    dprintf("\n");
    */

    fwrite(dip_switches.data, 1, 0x6000, f);

    fclose(f);
}
