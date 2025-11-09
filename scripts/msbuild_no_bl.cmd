@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x86
set "VCTargetsPath=C:\Program Files (x86)\MSBuild\Microsoft.Cpp\v4.0"
echo Using VCTargetsPath=%VCTargetsPath%
"C:\Windows\Microsoft.NET\Framework64\v4.0.30319\MSBuild.exe" "E:\workspace\AutoBBS\build\vs2010\AsirikuyFramework.sln" /p:Configuration=Debug /m:1 /p:TrackFileAccess=false /p:VCTargetsPath="%VCTargetsPath%" /nologo /verbosity:minimal > E:\workspace\AutoBBS\msbuild_no_bl_output.log 2>&1
echo MSBUILD_EXIT=%ERRORLEVEL%
