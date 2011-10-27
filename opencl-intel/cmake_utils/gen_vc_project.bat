@echo off
setlocal

rem
rem Build Visual Studio 9 2008 projects for OpenCL
rem
rem Usage:
rem    gen_vc_project [+cnf] [-cmrt] [+java] [+dbg] [-x64] [vc|intel] [build_path] [build_type]
rem
rem  +cnf           - include conformance tests into solution
rem  -cmrt          - remove Common Runtime from the solution
rem  +java          - include java code
rem  +dbg           - include debugger engine into solution	
rem  vc|intel       - use VC or Intel compiler. Default - VC.
rem  build_path     - working directory. Default - "build" dir in parallel to "src"
rem  build_type     - debug or release.
rem  -x64 	        - build 64-bit version (default is 32-bit)
rem
rem  VC project is created in the directory build_path parallel to top level src directory
rem  Compilation output during build is copied to the directory bin\Debug or bin\Release
rem  parallel to the top level src directory
rem

set script_dir=%~dp0
cd "%script_dir%\..\..\"

set top_dir= %CD%


set incl_conf=OFF
set incl_cmrt=ON
set incl_java=OFF
set incl_dbg=OFF
set use_x64=OFF
set use_vc=1
set build_path=
set build_type=

:params_loop
    if "%1" == "" goto exit_params_loop

	if x%1 == x+cnf (
		set incl_cnf=ON
		echo Include CNF
	) else if x%1 == x-cmrt (
		set incl_cmrt=OFF
		echo Include GEN
	) else if x%1 == x+java (
		set incl_java=ON
		echo Include Java
	) else if x%1 == x+dbg (
		set incl_dbg=ON
		echo Debug mode
	) else if x%1 == x-x64 (
		set use_x64=ON
		echo 64bit mode
	) else if x%1 == xintel (
		set use_vc=0
		echo Use Intel compiler
	) else if x%1 == xvc (
		set use_vc=1
		echo Use Visual compiler
	) else (
		if x%build_path% == x (
			set build_path=%1
		) else if x%build_type% == x (
			set build_type=%~1
		)
	)
	
    shift
	goto params_loop
:exit_params_loop
if x%build_path% == x (
	set build_path=build
)
rem CMAKE_BUILD_TYPE is case-sensitive, it must be either "Derbug" or "Release", not "debug" or
rem "release". 
if x%build_type% == x (
	set build_type=Release
) else if x%build_type% == xrelease (
	set build_type=Release
) else if x%build_type% == xRelease (
	set build_type=Release
) else if x%build_type% == xdebug (
	set build_type=Debug
) else if x%build_type% == xDebug (
	set build_type=Debug
) else (
	echo %0: Bad build type: "%build_type%"; must be either "debug" or "release".
	exit /b 1
)

echo BUILD PATH: "%build_path%"
echo BUILD TYPE: "%build_type%"

if not exist %build_path% mkdir %build_path%

cd /D %build_path%

set GEN_VERSION="Visual Studio 9 2008"
set BUILD_CONFIG=win32
if %use_x64% == ON set GEN_VERSION="Visual Studio 9 2008 Win64"
if %use_x64% == ON  call "%VS90COMNTOOLS:\=/%/../../VC/bin/x86_amd64/vcvarsx86_amd64.bat"  
if %use_x64% == ON set BUILD_CONFIG=win64

set conformance_list=test_allocations test_api test_atomics test_basic test_buffers test_commonfns test_compiler computeinfo contractions test_conversions test_events test_geometrics test_gl test_d3d9 test_half test_headers test_cl_h test_cl_platform_h test_cl_gl_h test_opencl_h test_cl_copy_images test_cl_get_info test_cl_read_write_images test_kernel_image_methods test_image_streams test_integer_ops bruteforce test_multiples test_profiling test_relationals test_select test_thread_dimensions test_vecalign test_vecstep

cmake -G %GEN_VERSION% -D PYTHON_EXECUTABLE="C:\Python27\python.exe" -D INCLUDE_CONFORMANCE_TESTS=%incl_cnf% -D INCLUDE_CMRT=%incl_cmrt% -D BUILD_JAVA=%incl_java% -D INCLUDE_DEBUGGER=%incl_dbg% -D CONFORMANCE_LIST="%conformance_list%" -D BUILD_X64=%use_x64% -D CMAKE_BUILD_TYPE=%build_type%  -DCMAKE_INSTALL_PREFIX:PATH=%CD%/../../install/%BUILD_CONFIG%/\${BUILD_TYPE}/  %top_dir%\src
if not errorlevel 0 goto error_end

echo -- Fix C# projects referencies
python "%script_dir%\c++_2_c#.py" OCL.sln iocgui
if %incl_dbg% == ON python "%script_dir%\c++_2_c#.py" OCL.sln OCLDebugEngine OCLDebugConfigPackage

echo -- Convert relevant projects to Intel C++, please wait...
set converter="C:\Program Files (x86)\Common Files\Intel\shared files\ia32\Bin\ICProjConvert110.exe"

if %use_vc% == 0 goto use_intel
REM set intel_subset=clangSema clang clang_compiler

REM set builtins=clbltfnn8 clbltfnp8 clbltfnt7 clbltfnv8 clbltfnh8 clbltfny8 clbltfne7 clbltfnu8 clbltfng9 clbltfne9
%converter% OCL.sln %intel_subset% %conformance_list% /IC /nologo /q
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

