@echo off
setlocal

set VS_exe=%VS90COMNTOOLS%\..\IDE\devenv.exe

rem
rem Build MinGW makefiles to compile OpenCL on Windows and run make
rem
rem Usage:
rem    build_project [debug|release [intel]]
rem
rem    debug|release   - build Debug or Release binaries. Default - debug
rem    intel           - use Intel Compiler. Default - Microsoft VC 2008 32 bit
rem
rem  Makefiles are created in the directory cmd_build parallel to top level src directory
rem  Output is copied to the directory bin\Debug or bin\Release parallel to the top level
rem  src directory
rem

if x%1==xrelease goto set_release
    set target=Debug
    goto after_target
:set_release
  set target=Release

:after_target
set comp_type=vc
if x%2==xintel set comp_type=intel

set compile_dir= "%~dp0\..\..\cmd_build_%comp_type%"

call "%~dp0\gen_vc_project.bat" %comp_type% %compile_dir% %target%

cd %compile_dir%

echo .
echo . Starting build %target%
echo . Log file: %CD%\build_%target%.log
echo .
"%VS_exe%" OCL.sln /build %target% /Out %CD%\build_%target%.log
