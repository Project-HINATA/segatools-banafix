@echo off

pushd %~dp0

start inject.exe -d -k idachook.dll amdaemon.exe -c config_common.json config_jp.json config_seat_1_jp.json config_seat_2_jp.json config_ex.json config_seat_1_ex.json config_seat_2_ex.json
inject -d -k idachook.dll ../WindowsNoEditor/GameProject/Binaries/Win64/GameProject-Win64-Shipping.exe

echo.
echo Game processes have terminated
pause