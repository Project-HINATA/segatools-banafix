#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <intrin.h>  
#include "util/dprintf.h"
#include "platform/opensslpatch.h"

int check_cpu() {
    int cpui[4] = {0};

    __cpuid(cpui, 0);
    int nIds_ = cpui[0];

    char vendor[0x20] = {0};
    *((int*)vendor) = cpui[1];
    *((int*)(vendor + 4)) = cpui[3];
    *((int*)(vendor + 8)) = cpui[2];

    int isIntel = (strcmp(vendor, "GenuineIntel") == 0);

    if (isIntel && nIds_ >= 7) {
        __cpuidex(cpui, 7, 0);
        return (cpui[1] & (1 << 29)) != 0; 
    }

    return 0;
}

static void openssl_patch(void) {
    const char* variablename = "OPENSSL_ia32cap";
    const char* variablevalue = "~0x20000000";  

    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Environment", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        if (RegSetValueExA(hKey, variablename, 0, REG_SZ, (const BYTE*)variablevalue, strlen(variablevalue) + 1) == ERROR_SUCCESS) {
            dprintf("OpenSSL Patch: Applied successfully, set the user environment variable %s to %s\n", variablename, variablevalue);
        } else {
            dprintf("OpenSSL Patch: Error: Failed to set the user environment variable.\n");
        }

        RegCloseKey(hKey);

        SendMessageTimeoutA(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)"Environment", SMTO_ABORTIFHUNG, 5000, NULL);
    } else {
        dprintf("OpenSSL Patch: Error: Failed to open the user environment registry key.\n");
    }
}

HRESULT openssl_patch_apply(const struct openssl_patch_config *cfg) {  
    HRESULT hr;

    assert(cfg != NULL);

    if (!cfg->enable) {
        return S_FALSE;
    }

    if (check_cpu()) {
        openssl_patch();
    }

    return S_OK;
}
