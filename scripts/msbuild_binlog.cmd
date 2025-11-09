@echo off
REM Run MSBuild with binary log and then a tiny cl.exe test.
call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x86
set "VCTargetsPath=C:\Program Files (x86)\MSBuild\Microsoft.Cpp\v4.0"
echo Using VCTargetsPath=%VCTargetsPath%
"C:\Windows\Microsoft.NET\Framework64\v4.0.30319\MSBuild.exe" "E:\workspace\AutoBBS\build\vs2010\AsirikuyFramework.sln" /p:Configuration=Debug /m:1 /p:TrackFileAccess=false /p:VCTargetsPath="%VCTargetsPath%" /bl:E:\workspace\AutoBBS\msbuild_full.binlog /nologo /verbosity:minimal > E:\workspace\AutoBBS\msbuild_full_output.log 2>&1
echo MSBUILD_EXIT=%ERRORLEVEL%
echo --- now testing cl.exe with a tiny C file ---
if not exist E:\workspace\AutoBBS\tmp mkdir E:\workspace\AutoBBS\tmp
echo int main(){return 0;} > E:\workspace\AutoBBS\tmp\cl_test.c
"C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\bin\cl.exe" /c E:\workspace\AutoBBS\tmp\cl_test.c > E:\workspace\AutoBBS\cl_test_output.log 2>&1
echo CL_EXIT=%ERRORLEVEL%
