@echo off

pushd %~dp0

start "AM Daemon" /min cmd /C timeout 10 ^& inject_x64 -d -k kemonohook_x64.dll ../amdaemon.exe -f -c ../config.json"
inject_x86 -d -k kemonohook_x86.dll Parade -screen-fullscreen 0 -popupwindow -screen-width 720 -screen-height 1280 -silent-crashes

taskkill /f /im amdaemon.exe > nul 2>&1

echo.
echo Game processes have terminated
pause