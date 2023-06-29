@echo off

pushd %~dp0

REM start /min inject.exe -d -k idachook.dll amdaemon.exe -f -c config_aime_high_ex.json config_aime_high_jp.json config_aime_normal_ex.json config_aime_normal_jp.json config_common.json config_ex.json config_jp.json config_laninstall_client_ex.json config_laninstall_client_jp.json config_laninstall_server_ex.json config_laninstall_server_jp.json config_seat_1_ex.json config_seat_1_jp.json config_seat_2_ex.json config_seat_2_jp.json config_seat_3_ex.json config_seat_3_jp.json config_seat_4_ex.json config_seat_4_jp.json config_seat_single_ex.json config_seat_single_jp.json
start /min inject -d -k idachook.dll amdaemon.exe -f -c config_aime_normal_jp.json config_common.json config_jp.json config_laninstall_server_jp.json config_seat_1_jp.json
inject -d -k idachook.dll ..\WindowsNoEditor\GameProject.exe -culture=ja launch=Cabinet ABSLOG="..\..\..\..\..\Userdata\GameProject.log" -Master -UserDir="..\..\..\Userdata" -NotInstalled -UNATTENDED
taskkill /f /im amdaemon.exe > nul 2>&1

echo.
echo Game processes have terminated
pause