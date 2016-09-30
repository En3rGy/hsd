@echo off

set MINGW_BIN=C:\Qt\5.5\mingw492_32\bin

echo Copying files...

set ProgName=hsd
set ProgExeName=hsd.exe
xcopy .\bin ..\%ProgName%_win32\bin /E /I /Y
xcopy .\doc ..\%ProgName%_win32\doc /E /I /Y
xcopy .\etc ..\%ProgName%_win32\etc /E /I /Y

REM Copy dependencies
REM xcopy .\3rd_parties\picoLog\PL1000.dll ..\StatusIndicator_win32\bin /E /I /Y

set VCINSTALLDIR=
set path=%path%;"%MINGW_BIN%"
%MINGW_BIN%\windeployqt %~dp0\bin\%ProgExeName% --dir %~dp0\..\%ProgName%_win32\bin --release

xcopy %MINGW_BIN%\libgcc_s_dw2-1.dll ..\%ProgName%_win32\bin /E /I /Y
xcopy %MINGW_BIN%\libwinpthread-1.dll ..\%ProgName%_win32\bin /E /I /Y
xcopy %MINGW_BIN%\libstdc++-6.dll ..\%ProgName%_win32\bin /E /I /Y

cd ..\%ProgName%_win32\bin

echo Deleting debug files
REM del *d.* /Q /S
REM del *d4.* /Q /S
REM del *d5.* /Q /S

pause
