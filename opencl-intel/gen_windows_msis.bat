@echo off
%~d0
set CUR_DIR=%~dp0

call perl -v | find "5.10.1">nul
if %ERRORLEVEL% NEQ 0 goto PERLNE
cd "%ProgramFiles%\Windows Installer XML\bin"
if %ERRORLEVEL% NEQ 0 goto WIXNE
cd "%CUR_DIR%..\BuildSystem"
if %ERRORLEVEL% NEQ 0 goto BSNE
call build.bat -bt opencl11_create_msis -p -napz -c
if %ERRORLEVEL% NEQ 0 goto BUILDFAIL
exit %ERRORLEVEL%

:PERLNE
echo ActivePerl v5.10.1 is not currently installed and it's required for msi's generations. Please install it from "\\nntavc101xwb1.ccr.corp.intel.com\AVC.QA_OpenCL_resources\OpenCL Tools".
exit 1

:WIXNE
echo Directory "%ProgramFiles%\Windows Installer XML\bin" doesn't exist. Please install WIX v2.0 to it. Which can be obtained from "\\nntavc101xwb1.ccr.corp.intel.com\AVC.QA_OpenCL_resources\OpenCL Tools\WIX Tools\Windows Installer XML".
exit 1

:BSNE
echo Directory "%CUR_DIR%..\BuildSystem" doesn't exist, please sync it from svn.
exit 1

:BUILDFAIL
echo Please see more details in "%CUR_DIR%..\BuildSystem\logs\Summary.log".
exit 1
