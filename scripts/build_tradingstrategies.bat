@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
msbuild "E:\workspace\AutoBBS\build\vs2010\projects\TradingStrategies.vcxproj" /p:Configuration=Debug /p:Platform=Win32 /p:PlatformToolset=v143 /p:TrackFileAccess=false /v:minimal
exit /b %ERRORLEVEL%
