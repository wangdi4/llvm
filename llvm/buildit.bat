@setlocal

set HERE=%~dp0

set THERE=%HERE%..

set BLDDIR=%THERE%\llvm_build_dir
set INSDIR=%THERE%\llvm_install_dir
set SRCDIR=%THERE%\llvm

cd %THERE%

@if not exist %BLDDIR%\. mkdir %BLDDIR%
@if not exist %INSDIR%\. mkdir %INSDIR%

@if exist %BLDDIR%\. cd %BLDDIR%

cmake -DCMAKE_INSTALL_PREFIX=%INSDIR% %SRCDIR%

@set INSDIR=
@set SRCDIR=
@set HERE=

@endlocal


