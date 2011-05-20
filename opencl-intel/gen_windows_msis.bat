@echo off
%~d0
set CUR_DIR=%~dp0

call perl -v | find "5.10.1">nul
if %ERRORLEVEL% NEQ 0 goto PERLNE
cd "%CUR_DIR%..\BuildSystem"
if %ERRORLEVEL% NEQ 0 goto BSNE
call perl gen_windows_msis.pl %*
if %ERRORLEVEL% NEQ 0 goto BUILDFAIL
exit /B %ERRORLEVEL%

:PERLNE
echo ActivePerl v5.10.1 is not currently installed and it's required for msi's generations. Please install it from "\\ger\ec\proj\ha\ptl\MobileTV\Installations\ActivePerl" or from "\\nntavc101xwb1.ccr.corp.intel.com\AVC.QA_OpenCL_resources\OpenCL Tools\ActivePerl".
exit /B 1

:BSNE
echo Directory "%CUR_DIR%..\BuildSystem" doesn't exist, please sync it from svn.
exit /B 1

:BUILDFAIL
echo Please see more details in "%CUR_DIR%..\BuildSystem\logs\Summary.log".
exit /B 1
