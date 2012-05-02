@echo off
setlocal

rem
rem Build Visual Studio 9 2008 projects for OpenCL
rem
rem Usage:
rem    gen_vc_project [-vs2010] [+smpls] [+cnf] [+cnf12] [-cmrt] [+java] [+dbg] [-x64] [-single] [vc|intel] [build_path] [build_type]
rem
rem  -vs2010        - create a Visual Studio 2010 solution 
rem  +smpls         - include samples tests
rem  +cnf           - include conformance 1.1 tests into solution
rem  +cnf12         - include conformance 1.2 tests into solution
rem  -cmrt          - remove Common Runtime from the solution
rem  +java          - include java code
rem  +dbg           - include debugger engine into solution	
rem  vc|intel       - use VC or Intel compiler. Default - VC.
rem  build_path     - working directory. Default - "build" dir in parallel to "src"
rem  build_type     - debug or release.
rem  -x64 	        - build 64-bit version (default is 32-bit)
rem  -single        - create single solution (do not create a separate solution for Conformance)
rem  -double        - create double solution (create a separate solution for Conformance)
rem
rem  VC project is created in the directory build_path parallel to top level src directory
rem  Compilation output during build is copied to the directory bin\Debug or bin\Release
rem  parallel to the top level src directory
rem

set script_dir=%~dp0
cd "%script_dir%\..\..\"

set top_dir= %CD%


set use_vs2010=OFF
set incl_smpls=OFF
set incl_cnf=OFF
set incl_cnf12=OFF
set incl_cmrt=ON
set incl_java=OFF
set incl_dbg=OFF
set use_x64=OFF
set use_vc=1
set conformance_mode=
set build_path=
set build_type=

:params_loop
    if "%1" == "" goto exit_params_loop

	if x%1 == x+cnf (
		set incl_cnf=ON
		echo Include CNF
	) else if x%1 == x+cnf12 (
		set incl_cnf12=ON
		echo Include CNF 1.2
	) else if x%1 == x-cmrt (
		set incl_cmrt=OFF
		echo Include GEN
    ) else if x%1 == x+smpls (
		set incl_smpls=ON
		echo Include Samples
    ) else if x%1 == x-vs2010 (
		set use_vs2010=ON
		echo Use Visual Studio 2010
	) else if x%1 == x+java (
		set incl_java=ON
		echo Include Java
	) else if x%1 == x+dbg (
		set incl_dbg=ON
		echo Debug mode
	) else if x%1 == x-x64 (
		set use_x64=ON
		echo 64bit mode
    ) else if x%1 == x-single (
        set conformance_mode=1
        echo Generate single solution
    ) else if x%1 == x-double (
        set conformance_mode=2
        echo Generate separate Conformance solution
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

if "%conformance_mode%-%incl_cnf%-%incl_cnf12%" == "1-ON-ON" (
	echo %0: In single soultion mode either Conformance 1.1 or 1.2 could be included, not both.
	exit /b 1
)

echo BUILD PATH: "%build_path%"
echo BUILD TYPE: "%build_type%"

if not exist %build_path% mkdir %build_path%

cd /D %build_path%

set BUILD_CONFIG=win32

if %use_vs2010% == ON (
    set GEN_VERSION="Visual Studio 10"
    if %use_x64% == ON set GEN_VERSION="Visual Studio 10 Win64"
    if %use_x64% == ON  call "%VS100COMNTOOLS:\=/%/../../VC/bin/x86_amd64/vcvarsx86_amd64.bat"
) else (
    set GEN_VERSION="Visual Studio 9 2008"
    if %use_x64% == ON set GEN_VERSION="Visual Studio 9 2008 Win64"
    if %use_x64% == ON  call "%VS90COMNTOOLS:\=/%/../../VC/bin/x86_amd64/vcvarsx86_amd64.bat"  
)

if %use_x64% == ON set BUILD_CONFIG=win64

set conformance_list=^
    clconf_harness ^
    bruteforce ^
    native_bruteforce ^
    relaxed_bruteforce ^
    computeinfo ^
    contractions ^
    test_allocations ^
    test_api ^
    test_atomics ^
    test_basic ^
    test_buffers ^
    test_cl_copy_images ^
    test_cl_fill_images ^
    test_cl_get_info ^
    test_cl_gl_h ^
    test_cl_h ^
    test_cl_platform_h ^
    test_cl_read_write_images ^
    test_commonfns ^
    test_compiler ^
    test_conversions ^
    test_device_partition ^
    test_events ^
    test_geometrics ^
    test_d3d9 ^
    test_d3d10 ^
    test_d3d11 ^
    test_gl ^
    test_headers ^
    test_half ^
    test_image_streams ^
    test_integer_ops ^
    test_kernel_image_methods ^
    test_media_surface_sharing ^
    test_mem_host_flags ^
    test_multiples ^
    test_opencl_h ^
    test_printf ^
    test_profiling ^
    test_relationals ^
    test_samplerless_reads ^
    test_select ^
    test_thread_dimensions ^
    test_vecalign ^
    test_vecstep

cmake ^
    -G %GEN_VERSION% ^
    -D PYTHON_EXECUTABLE="C:\Python27\python.exe" ^
    -D INCLUDE_SMPLS=%incl_smpls% ^
    -D CONFORMANCE_MODE=%conformance_mode% ^
    -D INCLUDE_CONFORMANCE_TESTS=%incl_cnf% ^
    -D INCLUDE_CONFORMANCE_1_2_TESTS=%incl_cnf12% ^
    -D INCLUDE_CMRT=%incl_cmrt% ^
    -D BUILD_JAVA=%incl_java% ^
    -D INCLUDE_DEBUGGER=%incl_dbg% ^
    -D CONFORMANCE_LIST="%conformance_list%" ^
    -D BUILD_X64=%use_x64% ^
    -D CMAKE_BUILD_TYPE=%build_type%  ^
    -DLLVM_USE_INTEL_JITEVENTS:BOOL=ON ^
    -DCMAKE_INSTALL_PREFIX:PATH=%CD%/../install/%BUILD_CONFIG%/\${BUILD_TYPE}/ ^
    %top_dir%\src

if not errorlevel 0 goto error_end

echo -- Fix C# projects referencies
python "%script_dir%\c++_2_c#.py" OCL.sln iocgui
if %incl_dbg% == ON python "%script_dir%\c++_2_c#.py" OCL.sln OCLDebugEngine OCLDebugConfigPackage

echo -- Convert relevant projects to Intel C++, please wait...
set converter="C:\Program Files (x86)\Common Files\Intel\shared files\ia32\Bin\ICProjConvert121.exe"

if %use_vc% == 0 goto use_intel
REM set intel_subset=clangSema clang clang_compiler

set conformance11_path=%build_path%\tests\conform1.1\src\test_conformance
set conformance12_path="%build_path%\tests\conform1.2\src\test_conformance"

set conformance11_project_list=^
    %top_dir%\%conformance11_path%\math_brute_force\bruteforce.vcxproj ^
	%top_dir%\%conformance11_path%\native_brute_force\native_bruteforce.vcxproj ^
	%top_dir%\%conformance11_path%\relaxed_brute_force\relaxed_bruteforce.vcxproj ^
    %top_dir%\%conformance11_path%\computeinfo\computeinfo.vcxproj ^
    %top_dir%\%conformance11_path%\contractions\contractions.vcxproj ^
    %top_dir%\%conformance11_path%\allocations\test_allocations.vcxproj ^
    %top_dir%\%conformance11_path%\api\test_api.vcxproj ^
    %top_dir%\%conformance11_path%\atomics\test_atomics.vcxproj ^
    %top_dir%\%conformance11_path%\basic\test_basic.vcxproj ^
    %top_dir%\%conformance11_path%\buffers\test_buffers.vcxproj ^
    %top_dir%\%conformance11_path%\images\clCopyImage\test_cl_copy_images.vcxproj ^
    %top_dir%\%conformance11_path%\images\clGetInfo\test_cl_get_info.vcxproj ^
    %top_dir%\%conformance11_path%\headers\test_cl_gl_h.vcxproj ^
    %top_dir%\%conformance11_path%\headers\test_cl_h.vcxproj ^
    %top_dir%\%conformance11_path%\headers\test_cl_platform_h.vcxproj ^
    %top_dir%\%conformance11_path%\images\clReadWriteImage\test_cl_read_write_images.vcxproj ^
    %top_dir%\%conformance11_path%\commonfns\test_commonfns.vcxproj ^
    %top_dir%\%conformance11_path%\compiler\test_compiler.vcxproj ^
    %top_dir%\%conformance11_path%\conversions\test_conversions.vcxproj ^
    %top_dir%\%conformance11_path%\events\test_events.vcxproj ^
    %top_dir%\%conformance11_path%\geometrics\test_geometrics.vcxproj ^
    %top_dir%\%conformance11_path%\d3d9\test_d3d9.vcxproj ^
    %top_dir%\%conformance11_path%\gl\test_gl.vcxproj ^
    %top_dir%\%conformance11_path%\headers\test_headers.vcxproj ^
    %top_dir%\%conformance11_path%\half\test_half.vcxproj ^
    %top_dir%\%conformance11_path%\images\test_read_write\test_image_streams.vcxproj ^
    %top_dir%\%conformance11_path%\integer_ops\test_integer_ops.vcxproj ^
    %top_dir%\%conformance11_path%\images\kernel_image_methods\test_kernel_image_methods.vcxproj ^
    %top_dir%\%conformance11_path%\multiple_device_context\test_multiples.vcxproj ^
    %top_dir%\%conformance11_path%\headers\test_opencl_h.vcxproj ^
    %top_dir%\%conformance11_path%\profiling\test_profiling.vcxproj ^
    %top_dir%\%conformance11_path%\relationals\test_relationals.vcxproj ^
    %top_dir%\%conformance11_path%\select\test_select.vcxproj ^
    %top_dir%\%conformance11_path%\thread_dimensions\test_thread_dimensions.vcxproj ^
    %top_dir%\%conformance11_path%\vec_align\test_vecalign.vcxproj ^
    %top_dir%\%conformance11_path%\vec_step\test_vecstep.vcxproj

set conformance12_project_list=^
    %top_dir%\%build_path%\tests\conform1.2\src\test_common\harness\clconf_harness.vcxproj ^
    %top_dir%\%conformance12_path%\math_brute_force\bruteforce.vcxproj ^
    %top_dir%\%conformance12_path%\computeinfo\computeinfo.vcxproj ^
    %top_dir%\%conformance12_path%\contractions\contractions.vcxproj ^
    %top_dir%\%conformance12_path%\allocations\test_allocations.vcxproj ^
    %top_dir%\%conformance12_path%\api\test_api.vcxproj ^
    %top_dir%\%conformance12_path%\atomics\test_atomics.vcxproj ^
    %top_dir%\%conformance12_path%\basic\test_basic.vcxproj ^
    %top_dir%\%conformance12_path%\buffers\test_buffers.vcxproj ^
    %top_dir%\%conformance12_path%\images\clCopyImage\test_cl_copy_images.vcxproj ^
    %top_dir%\%conformance12_path%\images\clFillImage\test_cl_fill_images.vcxproj ^
    %top_dir%\%conformance12_path%\images\clGetInfo\test_cl_get_info.vcxproj ^
    %top_dir%\%conformance12_path%\headers\test_cl_gl_h.vcxproj ^
    %top_dir%\%conformance12_path%\headers\test_cl_h.vcxproj ^
    %top_dir%\%conformance12_path%\headers\test_cl_platform_h.vcxproj ^
    %top_dir%\%conformance12_path%\images\clReadWriteImage\test_cl_read_write_images.vcxproj ^
    %top_dir%\%conformance12_path%\commonfns\test_commonfns.vcxproj ^
    %top_dir%\%conformance12_path%\compiler\test_compiler.vcxproj ^
    %top_dir%\%conformance12_path%\conversions\test_conversions.vcxproj ^
    %top_dir%\%conformance12_path%\device_partition\test_device_partition.vcxproj ^
    %top_dir%\%conformance12_path%\events\test_events.vcxproj ^
    %top_dir%\%conformance12_path%\geometrics\test_geometrics.vcxproj ^
    %top_dir%\%conformance12_path%\d3d9\test_d3d9.vcxproj ^ 
    %top_dir%\%conformance12_path%\d3d10\test_d3d10.vcxproj ^
    %top_dir%\%conformance12_path%\d3d11\test_d3d11.vcxproj ^
    %top_dir%\%conformance12_path%\gl\test_gl.vcxproj ^
    %top_dir%\%conformance12_path%\headers\test_headers.vcxproj ^
    %top_dir%\%conformance12_path%\half\test_half.vcxproj ^
    %top_dir%\%conformance12_path%\images\test_read_write\test_image_streams.vcxproj ^
    %top_dir%\%conformance12_path%\integer_ops\test_integer_ops.vcxproj ^
    %top_dir%\%conformance12_path%\images\kernel_image_methods\test_kernel_image_methods.vcxproj ^
    %top_dir%\%conformance12_path%\mem_host_flags\test_mem_host_flags.vcxproj ^
    %top_dir%\%conformance12_path%\multiple_device_context\test_multiples.vcxproj ^
    %top_dir%\%conformance12_path%\headers\test_opencl_h.vcxproj ^
    %top_dir%\%conformance12_path%\printf\test_printf.vcxproj ^
    %top_dir%\%conformance12_path%\profiling\test_profiling.vcxproj ^
    %top_dir%\%conformance12_path%\relationals\test_relationals.vcxproj ^
    %top_dir%\%conformance12_path%\images\samplerlessReads\test_samplerless_reads.vcxproj ^
    %top_dir%\%conformance12_path%\select\test_select.vcxproj ^
    %top_dir%\%conformance12_path%\thread_dimensions\test_thread_dimensions.vcxproj ^
    %top_dir%\%conformance12_path%\vec_align\test_vecalign.vcxproj ^
    %top_dir%\%conformance12_path%\vec_step\test_vecstep.vcxproj
    
REM set builtins=clbltfnn8 clbltfnp8 clbltfnt7 clbltfnv8 clbltfnh8 clbltfny8 clbltfne7 clbltfnu8 clbltfng9 clbltfne9
REM echo %converter% OCL.sln %intel_subset% %conformance_list% /IC:"Intel C++ Compiler XE 12.1" /nologo /q
echo %converter% %conformance11_project_list% /IC:"Intel C++ Compiler XE 12.1" /nologo /q
echo %incl_conf%

%converter% %conformance11_project_list% /IC:"Intel C++ Compiler XE 12.1" /nologo /q
echo converted conformance 1.1 projects to ICC 12.1


if %incl_conf12% == ON (
%converter% %conformance12_project_list% /IC:"Intel C++ Compiler XE 12.1" /nologo /q
echo converted conformance 1.2 projects to ICC 12.1
)
REM %converter% OCL.sln test_atomics.vcxproj /IC:"Intel C++ Compiler XE 12.1" /nologo /q

goto end

:use_intel
%converter% OCL.sln /IC /nologo /q
goto end

:end
echo .
echo . %GEN_VERSION% project created:
echo . %CD%\OCL.sln
echo .

:error_end

