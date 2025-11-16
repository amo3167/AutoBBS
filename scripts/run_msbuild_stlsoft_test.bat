@echo off
REM Quick build test after STLSoft patch
call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch=x86 -host_arch=x64
cd /d E:\workspace\AutoBBS
msbuild build\vs2010\AsirikuyFramework.sln /m:1 /p:Configuration=Debug /p:PlatformToolset=v143 /p:WindowsTargetPlatformVersion=10.0.26100.0 /p:TrackFileAccess=false > msbuild_stlsoft_patched.log 2>&1
echo Build completed, check msbuild_stlsoft_patched.log
