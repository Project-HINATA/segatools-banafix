#include <windows.h>

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "hook/table.h"

static BOOL WINAPI my_CreateProcessA(
    LPCSTR                lpApplicationName,
    LPSTR                 lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL                  bInheritHandles,
    DWORD                 dwCreationFlags,
    LPVOID                lpEnvironment,
    LPCSTR                lpCurrentDirectory,
    LPSTARTUPINFOA        lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
);
BOOL my_CreateProcessW(
    LPCWSTR               lpApplicationName,
    LPWSTR                lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL                  bInheritHandles,
    DWORD                 dwCreationFlags,
    LPVOID                lpEnvironment,
    LPCWSTR               lpCurrentDirectory,
    LPSTARTUPINFOW        lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
);

static BOOL (WINAPI *next_CreateProcessA)(
    LPCSTR                lpApplicationName,
    LPSTR                 lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL                  bInheritHandles,
    DWORD                 dwCreationFlags,
    LPVOID                lpEnvironment,
    LPCSTR                lpCurrentDirectory,
    LPSTARTUPINFOA        lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
);

static BOOL (WINAPI *next_CreateProcessW)(
    LPCWSTR               lpApplicationName,
    LPWSTR                lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL                  bInheritHandles,
    DWORD                 dwCreationFlags,
    LPVOID                lpEnvironment,
    LPCWSTR               lpCurrentDirectory,
    LPSTARTUPINFOW        lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
);

static const struct hook_symbol win32_hooks[] = {
    {
        .name = "CreateProcessA",
        .patch = my_CreateProcessA,
        .link = (void **) &next_CreateProcessA
    },
    {
        .name = "CreateProcessW",
        .patch = my_CreateProcessW,
        .link = (void **) &next_CreateProcessW
    },
};

static bool did_init = false;

static struct process_hook_sym_w *processe_syms_w;
static struct process_hook_sym_a *processe_syms_a;

static size_t processe_nsyms_a = 0;
static size_t processe_nsyms_w = 0;

void createprocess_push_hook_w(const wchar_t *name, const wchar_t *dll_name, const wchar_t *tail) {
    createprocess_hook_init();
}

void createprocess_push_hook_a(const char *name, const char *dll_name, const char *tail) {
    createprocess_hook_init();
}

void createprocess_hook_init() {
    if (did_init) {
        return;
    }
    did_init = true;

    hook_table_apply(
            NULL,
            "kernel32.dll",
            win32_hooks,
            _countof(win32_hooks));

    dprintf("CreateProcess: Init\n");
}


static BOOL WINAPI my_CreateProcessA(
    LPCSTR                lpApplicationName,
    LPSTR                 lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL                  bInheritHandles,
    DWORD                 dwCreationFlags,
    LPVOID                lpEnvironment,
    LPCSTR                lpCurrentDirectory,
    LPSTARTUPINFOA        lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
)
{
    if (strncmp(".\\15312firm\\firmupdate_1113.exe", lpCommandLine, 31)) {
        return next_CreateProcessA(
            lpApplicationName,
            lpCommandLine,
            lpProcessAttributes,
            lpThreadAttributes,
            bInheritHandles,
            dwCreationFlags,
            lpEnvironment,
            lpCurrentDirectory,
            lpStartupInfo,
            lpProcessInformation
        );
    }

    dprintf("CreateProcess: Hooking child process %s\n", lpCommandLine);
    char new_cmd[MAX_PATH] = "inject -d -k carolhook.dll ";
    strcat_s(new_cmd, MAX_PATH, lpCommandLine);

    return next_CreateProcessA(
            lpApplicationName,
            new_cmd,
            lpProcessAttributes,
            lpThreadAttributes,
            bInheritHandles,
            dwCreationFlags,
            lpEnvironment,
            lpCurrentDirectory,
            lpStartupInfo,
            lpProcessInformation
        );
}

BOOL my_CreateProcessW(
    LPCWSTR               lpApplicationName,
    LPWSTR                lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL                  bInheritHandles,
    DWORD                 dwCreationFlags,
    LPVOID                lpEnvironment,
    LPCWSTR               lpCurrentDirectory,
    LPSTARTUPINFOW        lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation)
{
    return next_CreateProcessW(
        lpApplicationName,
        lpCommandLine,
        lpProcessAttributes,
        lpThreadAttributes,
        bInheritHandles,
        dwCreationFlags,
        lpEnvironment,
        lpCurrentDirectory,
        lpStartupInfo,
        lpProcessInformation
    );
}