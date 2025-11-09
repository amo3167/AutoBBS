@echo off
call "E:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
cd /d E:\workspace\AutoBBS
msbuild build\vs2010\AsirikuyFramework.sln /p:Configuration=Debug /p:Platform=Win32 /p:PlatformToolset=v143 /p:WindowsTargetPlatformVersion=10.0.26100.0 /p:TrackFileAccess=false /m:1 /t:NTPClient /v:m
