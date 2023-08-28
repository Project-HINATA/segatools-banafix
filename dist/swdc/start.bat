@echo off

pushd %~dp0

REM set the APP_DIR to the Y drive
set APP_DIR=Y:\SWDC

REM create the APP_DIR if it doesn't exist and redirect it to the TEMP folder
if not exist "%APP_DIR%" (
    subst Y: %TEMP%
    REM timeout /t 1
    if not exist "%APP_DIR%" (
        mkdir "%APP_DIR%"
    )
)

echo Mounted the Y:\ drive to the %TEMP%\SWDC folder

REM start /min inject -d -k swdchook.dll amdaemon.exe -c config.json config_LanClient.json config_MiniCabinet.json config_hook.json
start /min inject -d -k swdchook.dll amdaemon.exe -c config.json config_LanServer.json config_MiniCabinet.json config_hook.json
inject -d -k swdchook.dll ..\Todoroki\Binaries\Win64\Todoroki-Win64-Shipping.exe -launch=MiniCabinet -ABSLOG="..\..\..\..\..\Userdata\Todoroki.log" -UserDir="..\..\..\Userdata" -NotInstalled -UNATTENDED
taskkill /f /im amdaemon.exe > nul 2>&1

REM unmount the APP_DIR
subst Y: /d > nul 2>&1

echo.
echo Game processes have terminated
pause