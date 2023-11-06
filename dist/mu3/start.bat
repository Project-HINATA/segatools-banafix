@echo off
pushd %~dp0

taskkill /f /im amdaemon.exe > nul 2>&1

start inject -d -k mu3hook.dll amdaemon.exe -f -c config_client.json config_common.json config_server.json
inject -d -k mu3hook.dll mu3.exe

taskkill /f /im amdaemon.exe > nul 2>&1

echo Game processes have terminated