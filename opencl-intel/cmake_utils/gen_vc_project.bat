@echo off
setlocal

rem
rem Build Visual Studio 9 2008 projects for OpenCL
rem
rem Usage:
rem    gen_vc_project [+cnf] [-x64] [vc|intel] [working_dir] 
rem
rem  +cnf           - include conformance tests into solution
rem  vc|intel       - use VC or Intel compiler. Default - VC.
rem  working_dir    - working directory. Default - "build" dir in parallel to "src"
rem  -x64 	    - build 64-bit version (default is 32-bit)
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
if x%1 == x+cnf shift

set use_x64=OFF
if x%1 == x-x64 set use_x64=ON
if x%1 == x-x64 shift

set use_vc=1
if x%1 == xintel set use_vc=0

set working_dir=build
if not x%2 == x set working_dir=%2

if not exist %working_dir% mkdir %working_dir%

cd %working_dir%

set GEN_VERSION="Visual Studio 9 2008"
if %use_x64% == ON set GEN_VERSION="Visual Studio 9 2008 Win64"
if %use_x64% == ON  call "%VS90COMNTOOLS:\=/%/../../VC/bin/x86_amd64/vcvarsx86_amd64.bat"  

set conformance_list=test_allocations test_api test_atomics test_basic test_buffers test_commonfns test_compiler computeinfo contractions test_conversions test_events test_geometrics test_gl test_half test_headers test_cl_h test_cl_platform_h test_cl_gl_h test_opencl_h test_cl_copy_images test_cl_get_info test_cl_read_write_images test_kernel_image_methods test_image_streams test_integer_ops bruteforce test_multiples test_profiling test_relationals test_select test_thread_dimensions test_vecalign test_vecstep

cmake -G %GEN_VERSION% -D INCLUDE_CONFORMANCE_TESTS=%incl_cnf% -D CONFORMANCE_LIST="%conformance_list%" -D BUILD_X64=%use_x64% %top_dir%\src

if not errorlevel 0 goto error_end

echo -- Fix C# projects referencies
python "%script_dir%\c++_2_c#.py" OCL.sln OfflineCompiler OpenCLTracer 

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

