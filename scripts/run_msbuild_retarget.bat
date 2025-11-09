@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
msbuild "E:\workspace\AutoBBS\build\vs2010\AsirikuyFramework.sln" /m:1 /p:Configuration=Debug /p:PlatformToolset=v143 /p:WindowsTargetPlatformVersion=10.0.26100.0 /p:TrackFileAccess=false > "E:\workspace\AutoBBS\msbuild_retarget_capture.log" 2>&1
exit /b %ERRORLEVEL%
