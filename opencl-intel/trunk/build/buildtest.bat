@echo off

if exist "c:\perl\bin\wperl.exe" goto build
echo Warning: can't locate ActivePerl files.  ActivePerl v5.10 or higher required.

goto error

:build

rd /q /s win32
if not %ERRORLEVEL%==0 goto error

rd /q /s win64
if not %ERRORLEVEL%==0 goto error

cd BuildSystem
call build.bat -bt icsc_btbat -p -c
if not %ERRORLEVEL%==0 goto error

echo BUILD SUCCEEDED SUCCESSFULLY
cd ..
goto end

:error
echo BUILD FAILED

:end
