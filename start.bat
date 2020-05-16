@echo off
:start
::Server name
set serverName=Jims DayZ server
::Server files location
set serverLocation="C:\Program Files (x86)\Steam\steamapps\common\DayZServer"
::BEC location
set BECLocation="C:\Program Files (x86)\Steam\steamapps\common\DayZServer\BEC"
::Server Port
set serverPort=2302
::Server config
set serverConfig=serverDZ.cfg
::Logical CPU cores to use (Equal or less than available)
set serverCPU=2
::Sets title for terminal (DONT edit)
title %serverName% batch
::DayZServer location (DONT edit)
cd "%serverLocation%"
echo (%time%) %serverName% started.
::Launch parameters (edit end: -config=|-port=|-profiles=|-doLogs|-adminLog|-netLog|-freezeCheck|-filePatching|-BEpath=|-cpuCount=)
start "DayZ Server" /min "DayZServer_x64.exe" -config=%serverConfig% -port=%serverPort% -cpuCount=%serverCPU% -dologs -adminlog -netlog -freezecheck
::Time in seconds before starting BEC
timeout 25
cd /d "%BECLocation%"
start "" "bec.exe"
::Time in seconds before kill server and BEC process (14400 = 4 hours)
timeout 14390
taskkill /im DayZServer_x64.exe /F
taskkill /im bec.exe /F
::Time in seconds to wait before..
timeout 10
::Go back to the top and repeat the whole cycle again
goto start