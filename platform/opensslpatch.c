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

static int is_env_variable_set(const char* variablename, const char* expectedvalue) {
    char currentvalue[256];
    DWORD length = GetEnvironmentVariableA(variablename, currentvalue, sizeof(currentvalue));

    if (length > 0 && length < sizeof(currentvalue)) {
        return strcmp(currentvalue, expectedvalue) == 0;
    }
    return 0;
}

//Set User's Environment variable via registry
static void openssl_patch(void) {
    const char* variablename = "OPENSSL_ia32cap";
    const char* variablevalue = "~0x20000000";

    if (is_env_variable_set(variablename, variablevalue)) {
        return;
    }

    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Environment", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        if (RegSetValueExA(hKey, variablename, 0, REG_SZ, (const BYTE*)variablevalue, strlen(variablevalue) + 1) == ERROR_SUCCESS) {
            dprintf("OpenSSL Patch: Applied successfully. User environment variable %s set to %s\n", variablename, variablevalue);
            SendMessageTimeoutA(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)"Environment", SMTO_ABORTIFHUNG, 5000, NULL);
            dprintf(
            "Please close this windows and reopen the game to enjoy.\n"
            );
            ExitProcess(0);
        } else {
            dprintf("OpenSSL Patch: Error: Failed to set the user environment variable.\n");
        }

        RegCloseKey(hKey);
    } else {
        dprintf("OpenSSL Patch: Error: Failed to open the user environment registry key.\n");
    }
}

//Set environment variable for current process
// static void openssl_patch(void) {
//     const wchar_t* variablename = L"OPENSSL_ia32cap";
//     const wchar_t* variablevalue = L"~0x20000000";

//     if (SetEnvironmentVariableW(variablename, variablevalue)) {
//         dprintf("OpenSSL Patch: Applied successfully, set the environment variable %ls to %ls\n", variablename, variablevalue);
//     } else {
//         dprintf("OpenSSL Patch: Error: Failed to set the environment variable.\n");
//     }
// }

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
