@echo off

pushd %~dp0

rem Matching Server
start /min ..\..\..\Tools\tdrserver.exe
REM start /min inject -d -k swdchook.dll amdaemon.exe -c config.json config_LanClient.json config_MiniCabinet.json config_hook.json
start /min inject -d -k swdchook.dll amdaemon.exe -c config.json config_LanServer.json config_MiniCabinet.json
REM Valid -launch parameters are "PC", "Cabinet" and "MiniCabinet
inject -d -k swdchook.dll ..\Todoroki\Binaries\Win64\Todoroki-Win64-Shipping.exe -launch=MiniCabinet -ABSLOG="..\..\..\..\..\Userdata\Todoroki.log" -UserDir="..\..\..\Userdata" -NotInstalled -UNATTENDED

taskkill /f /im amdaemon.exe > nul 2>&1
taskkill /f /im tdrserver.exe > nul 2>&1

echo.
echo Game processes have terminated
pause