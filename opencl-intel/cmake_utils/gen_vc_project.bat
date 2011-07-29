@echo off
setlocal

rem
rem Build Visual Studio 9 2008 projects for OpenCL
rem
rem Usage:
rem    gen_vc_project [+cnf] [+gen] [+java] [+dbg] [-x64] [vc|intel] [working_dir] [build_type]
rem
rem  +cnf           - include conformance tests into solution
rem  +gen           - include GEN device into solution
rem  +java          - include java code
rem  +dbg           - include debugger engine into solution	
rem  vc|intel       - use VC or Intel compiler. Default - VC.
rem  working_dir    - working directory. Default - "build" dir in parallel to "src"
rem  -x64 	        - build 64-bit version (default is 32-bit)
rem  build_type     - Debug or Release.
rem
rem  VC project is created in the directory working_dir parallel to top level src directory
rem  Compilation output during build is copied to the directory bin\Debug or bin\Release
rem  parallel to the top level src directory
rem

set script_dir=%~dp0
cd "%script_dir%\..\..\"

set top_dir= %CD%


set incl_conf=OFF
if x%1 == x+cnf set incl_cnf=ON
if x%1 == x+cnf shift /1

set incl_gen=OFF
if x%1 == x+gen set incl_gen=ON
if x%1 == x+gen shift /1

set incl_java=OFF
if x%1 == x+java set incl_java=ON
if x%1 == x+java shift /1

set incl_dbg=OFF
if x%1 == x+dbg set incl_dbg=ON
if x%1 == x+dbg shift /1

set use_x64=OFF
if x%1 == x-x64 set use_x64=ON
if x%1 == x-x64 shift /1

set use_vc=1
if x%1 == xintel set use_vc=0

set working_dir=build
if not x%2 == x set working_dir=%2

set build_type=
if "%~3" == ""        set build_type=Release
if "%~3" == "Debug"   set build_type=Debug
if "%~3" == "debug"   set build_type=Debug
if "%~3" == "Release" set build_type=Release
if "%~3" == "release" set build_type=Release
if "%build_type%" == "" ( echo %0: Unknown build type: "%~3". >&2 & exit /b 1 )

if not exist %working_dir% mkdir %working_dir%

cd %working_dir%

set GEN_VERSION="Visual Studio 9 2008"
if %use_x64% == ON set GEN_VERSION="Visual Studio 9 2008 Win64"
if %use_x64% == ON  call "%VS90COMNTOOLS:\=/%/../../VC/bin/x86_amd64/vcvarsx86_amd64.bat"  

set conformance_list=test_allocations test_api test_atomics test_basic test_buffers test_commonfns test_compiler computeinfo contractions test_conversions test_events test_geometrics test_gl test_half test_headers test_cl_h test_cl_platform_h test_cl_gl_h test_opencl_h test_cl_copy_images test_cl_get_info test_cl_read_write_images test_kernel_image_methods test_image_streams test_integer_ops bruteforce test_multiples test_profiling test_relationals test_select test_thread_dimensions test_vecalign test_vecstep

cmake -G %GEN_VERSION% -D INCLUDE_CONFORMANCE_TESTS=%incl_cnf% -D INCLUDE_GEN_DEVICE=%incl_gen% -D BUILD_JAVA=%incl_java% -D INCLUDE_DEBUGGER=%incl_dbg% -D CONFORMANCE_LIST="%conformance_list%" -D BUILD_X64=%use_x64% -D CMAKE_BUILD_TYPE=%build_type% %top_dir%\src

if not errorlevel 0 goto error_end

echo -- Fix C# projects referencies
python "%script_dir%\c++_2_c#.py" OCL.sln iocgui
if %incl_dbg% == ON python "%script_dir%\c++_2_c#.py" OCL.sln OCLDebugEngine OCLDebugConfigPackage

echo -- Convert relevant projects to Intel C++, please wait...
set converter="C:\Program Files (x86)\Common Files\Intel\shared files\ia32\Bin\ICProjConvert110.exe"

if %use_vc% == 0 goto use_intel
REM set intel_subset=clangSema clang clang_compiler

set builtins=clbltfnn8 clbltfnp8 clbltfnt7 clbltfnv8 clbltfnh8 clbltfny8 clbltfne7 clbltfnu8
%converter% OCL.sln %intel_subset% %conformance_list% %builtins% /IC /nologo /q
goto end

:use_intel
%converter% OCL.sln /IC /nologo /q
goto end

:end
echo .
echo . VC 2008 project created:
echo . %CD%\OCL.sln
echo .

:error_end

