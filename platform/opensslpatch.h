#pragma once

#include <windows.h>

int ChecknPatch(void);
void OpenSSLPatch(void);
char* GetCpuName(void);
int CheckCpu(char* cpuname);
