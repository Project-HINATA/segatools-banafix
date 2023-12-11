#pragma once

#include <windows.h>
#include <dinput.h>

#include <stdint.h>

union idac_di_state {
    DIJOYSTATE st;
    uint8_t bytes[sizeof(DIJOYSTATE)];
};

HRESULT idac_di_dev_start(IDirectInputDevice8W *dev, HWND wnd);
void idac_di_dev_start_fx(IDirectInputDevice8W *dev, IDirectInputEffect **out, uint16_t strength);
HRESULT idac_di_dev_poll(
        IDirectInputDevice8W *dev,
        HWND wnd,
        union idac_di_state *out);

