#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "util/dprintf.h"
#include "platform/opensslpatch.h"

static char* GetCpuName() {
    FILE* fp;
    char buffer[128];
    char* cpu_info = NULL;

    fp = _popen("wmic cpu get Name", "r");

    if (fp == NULL) {
        return NULL;
    }

    fgets(buffer, sizeof(buffer), fp);

    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
        cpu_info = (char*)malloc(strlen(buffer) + 1);  
        strcpy(cpu_info, buffer);  
    }
    _pclose(fp);

    if (cpu_info != NULL) {
        cpu_info[strcspn(cpu_info, "\r\n")] = 0;
    }

    return cpu_info;
}

static int CheckCpu(char* cpuname) {
    if (strstr(cpuname, "Core 2 Duo") || strstr(cpuname, "Core 2 Quad") ||
        (strstr(cpuname, "Pentium") && !strstr(cpuname, "G")) || strstr(cpuname, "Celeron")) {
        return 0;
    }

    if (strstr(cpuname, "Intel")) {
        char* part = strtok(cpuname, " ");
        while (part != NULL) {
            if (part[0] == 'i' && strlen(part) >= 4) {
                int gen = atoi(part + 1);  
                if (gen >= 10) {
                    dprintf("OpenSSL Patch: Intel Gen 10+ CPU Detected: %s\n", cpuname);
                    return 1;
                }
            } else if (part[0] == 'G' && strlen(part) >= 4) {
                int pentium = atoi(part + 1);
                if (pentium / 1000 >= 6) {
                    dprintf("OpenSSL Patch: Intel Gen 10+ CPU Detected: %s\n", cpuname);
                    return 1;
                }
            }
            part = strtok(NULL, " ");
        }
    }

    return 0;
}

static void OpenSSLPatch(void) {
    const char* variablename = "OPENSSL_ia32cap";
    const char* variablevalue = "~0x20000000";  

    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Environment", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        if (RegSetValueExA(hKey, variablename, 0, REG_SZ, (const BYTE*)variablevalue, strlen(variablevalue) + 1) == ERROR_SUCCESS) {
            dprintf("OpenSSL Patch: Applied successfully : Set the user environment variable %s to %s\n", variablename, variablevalue);
        } else {
            dprintf("OpenSSL Patch: Error: Failed to set the user environment variable.\n");
        }

        RegCloseKey(hKey);

        SendMessageTimeoutA(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)"Environment", SMTO_ABORTIFHUNG, 5000, NULL);
    } else {
        dprintf("OpenSSL Patch: Error: Failed to open the user environment registry key.\n");
    }
}

int openssl_patch_apply(void) {  
    char* cpuname = GetCpuName();
    if (cpuname == NULL) {
        dprintf("OpenSSL Patch: Error: Unable to detect CPU.\n");
        return 1;  
    }

    if (CheckCpu(cpuname)) {
        OpenSSLPatch();
    } 
    free(cpuname);  
    return 0;  
}