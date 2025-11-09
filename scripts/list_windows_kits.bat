@echo off
set OUTFILE=E:\workspace\AutoBBS\windows_kits_list.txt
echo Listing Windows Kits 10 Include folders > %OUTFILE%
if exist "C:\Program Files (x86)\Windows Kits\10\Include" (
  for /D %%d in ("C:\Program Files (x86)\Windows Kits\10\Include\*") do echo %%~nxd >> %OUTFILE%
) else (
  echo No Windows Kits Include found >> %OUTFILE%
)
type %OUTFILE%
