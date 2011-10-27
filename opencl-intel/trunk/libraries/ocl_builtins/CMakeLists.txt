#--------------------------- Settings ----------------------------------------------------------
#-- All Win32/Win64/Linux32/Linux64-dependent definitions should be placed here

#
#  Variables which override defaults (used only by OCL SDK build)
#			OCL_INSTALL_DIR			   overrides output directory. The default is where the rest of OCL CPU BE resides.
#			OCL_CLANG_HEADERS			 overrides directory name for clang headers. The default is 'clang_headers' in Volcano root
#     OCL_GLOBAL_INCLUDE		 overrides directory name for global headers. The default is 'cl_api' in Volcano root
#     OCL_SOURCE_DIR				 TRUE when invoked by OCL SDK build
#     OCL_HOST_CLANG         overrides 'clang' tool selection. The default is to use 'clang' tool from the same build
#     OCL_HOST_OPT					 overrides 'opt' tool selection. The default is to use 'opt' tool from the same build
#     OCL_HOST_LLVM_LINK		 overrides 'llvm-link' tool selection. The default is to use 'llvm-link' tool from the same build
#

#
# Global and platform settings
#
cmake_minimum_required(VERSION 2.6.3)
cmake_policy(SET CMP0001 NEW)
cmake_policy(SET CMP0011 OLD)

project(cl_builtins)

# target name prefix to be used for generated binaries 
set (TARGET_NAME_PREFIX clbltfn)

if (CMAKE_SIZEOF_VOID_P EQUAL 4)
	# 32 bit
	set (PLATFORM 32)
else ()
	# 64 bit
	set (PLATFORM 64)
endif ()

if (WIN32)
 set (PLATFORM Win${PLATFORM})
else ()
 set (PLATFORM Linux${PLATFORM})
endif ()
message ("OCL Builtins build: ${PLATFORM}")

# Build type selection (Debug or Release) follows Visual Studio (in Windows case) or LLVM build type (for Linux)
# IMPORTANT: once we will switch to Visual Studio 10, the ${CMAKE_CFG_INTDIR} comparison must be done vs. $(Configuration) rather than $(OutDir)!
if (WIN32)
    if (${CMAKE_CFG_INTDIR} STREQUAL $(Configuration))
        set (CL_BUILD_TYPE ${CMAKE_CFG_INTDIR})
    endif()	
    if (MSVC_VERSION EQUAL 1500 AND ${CMAKE_CFG_INTDIR} STREQUAL $(OutDir))
        set (CL_BUILD_TYPE ${CMAKE_CFG_INTDIR})
    endif()
else ()
    set (CL_BUILD_TYPE ${CMAKE_BUILD_TYPE})
endif ()

#
# Project directories
#

# Implicit:
#  cl_builtins_SOURCE_DIR - points to the top src directory
#  cl_builtins_BINARY_DIR - points to the build directory

# remember OCL builtins source dir
set (CL_BUILTIN_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
message ("Built-ins sources are in: ${CL_BUILTIN_SOURCE_DIR}")

# separate subdirs for each build type (Debug or Release)
set (INSTALL_SUBDIR ${CL_BUILD_TYPE})

# target directory for OCL .rtl, and .dll/.so of BI and SVML library
if (NOT DEFINED OCL_INSTALL_DIR)
	# adjustment of target directory to target platform
	if (WIN32)
		set (OCL_INSTALL_DIR ${LLVM_BINARY_DIR}/bin/${INSTALL_SUBDIR})
	else ()
		set (OCL_INSTALL_DIR ${LLVM_BINARY_DIR}/lib)
	endif()
endif (NOT DEFINED OCL_INSTALL_DIR)

set (BUILTINS_INSTALL_DIR ${OCL_INSTALL_DIR})
message ("Built-ins libraries will be in: ${BUILTINS_INSTALL_DIR}")

# global headers
if (DEFINED OCL_CLANG_HEADERS)
	set (CLANG_HEADERS ${OCL_CLANG_HEADERS})
else ()
	set (CLANG_HEADERS ${CL_BUILTIN_SOURCE_DIR}/../../clang_headers)
endif()
if (NOT EXISTS ${CLANG_HEADERS})
   message (FATAL_ERROR "Cannot find Clang headers (${CLANG_HEADERS})!")
endif()
message ("Clang headers are in: ${CLANG_HEADERS}")

if (DEFINED OCL_GLOBAL_INCLUDE)
	set (IMPORT_DIR ${OCL_GLOBAL_INCLUDE})
else ()
	set (IMPORT_DIR ${CL_BUILTIN_SOURCE_DIR}/../../cl_api )
endif ()
if (NOT EXISTS ${IMPORT_DIR})
   message (FATAL_ERROR "Cannot find OCL headers (${IMPORT_DIR})!")
endif()
message ("Global headers are in: ${IMPORT_DIR}")

# svml directories
if (WIN32)
	set (SVML_LIB_DIR ${CL_BUILTIN_SOURCE_DIR}/lib/${PLATFORM})
	if (NOT EXISTS ${SVML_LIB_DIR})
   	message (FATAL_ERROR "Cannot find SVML lib directory (${SVML_LIB_DIR})!")
	endif()
else ()
	set (SVML_LIB_DIR ${CL_BUILTIN_SOURCE_DIR}/bin/svml/${PLATFORM})
endif ()
set (SVML_DLL_DIR ${CL_BUILTIN_SOURCE_DIR}/bin/svml/${PLATFORM})
if (NOT EXISTS ${SVML_DLL_DIR})
   message (FATAL_ERROR "Cannot find SVML binary directory (${SVML_DLL_DIR})!")
endif()
set (SVML_NAME_PREFIX __ocl_svml_)
message ("SVML lib dir: ${SVML_LIB_DIR}")
message ("SVML binary dir: ${SVML_DLL_DIR}")

# LLVM tools are assumed to be pre-built within the global project
if (NOT DEFINED OCL_HOST_CLANG) 
	set (CLANG clang)
else()
  set (CLANG OCL_HOST_CLANG)
endif()
if (NOT DEFINED OCL_HOST_OPT) 
	set (LLVM_OPT opt)
else ()
	set (LLVM_OPT OCL_HOST_OPT)
endif()
if (NOT DEFINED OCL_HOST_LLVM_LINK) 
	set (LLVM_LINK llvm-link)
else()
  set (LLVM_LINK OCL_HOST_LLVM_LINK)
endif()

# LLVM triple is per OS and Arch
if (WIN32)	
	if (CMAKE_SIZEOF_VOID_P EQUAL 4)
		set (TRIPLE i686-pc-win32)
	else ()
		set (TRIPLE x86_64-pc-win32) 
	endif (CMAKE_SIZEOF_VOID_P EQUAL 4)	
else (WIN32)
  if (CMAKE_SIZEOF_VOID_P EQUAL 4)
  	set	(TRIPLE i686-pc-Linux)
  else ()
		set (TRIPLE amd64)
	endif (CMAKE_SIZEOF_VOID_P EQUAL 4) 
endif (WIN32)

#
# external tools
#

# icc executables and env scripts
if (CMAKE_SIZEOF_VOID_P EQUAL 4)
	set (ICC_PLATFORM ia32)
else ()
	set (ICC_PLATFORM intel64)
endif ()

if (WIN32)
  set (ICC_WIN32_PATH $ENV{ICPP_COMPILER11})
  # backup selection of ICC12 is commented out until it will be tested with Volcano
  #if (NOT DEFINED ICC_WIN32_PATH)
  #	set (ICC_WIN32_PATH $ENV{ICPP_COMPILER12})
  #endif ()
  if (NOT DEFINED ICC_WIN32_PATH)
  	message (FATAL_ERROR "ICC is not installed in the system")
  endif ()
 	set (ICC_ENV_SCRIPT ${ICC_WIN32_PATH}/bin/iclvars.bat)
 	if (NOT EXISTS ${ICC_ENV_SCRIPT})
 	  set (ICC_ENV_SCRIPT ${ICC_WIN32_PATH}/bin/${ICC_PLATFORM}/iclvars_${ICC_PLATFORM}.bat)
 	endif()
	set (ICC_CL ${ICC_WIN32_PATH}/bin/${ICC_PLATFORM}/icl.exe)
	set (ICC_LD ${ICC_WIN32_PATH}/bin/${ICC_PLATFORM}/xilink.exe)
else (WIN32)
  set (ICC_ENV_SCRIPT iccvars.sh)
  find_program (ICC_FOUND ${ICC_ENV_SCRIPT} NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH NO_CMAKE_FIND_ROOT_PATH)
  if (${ICC_FOUND} STREQUAL "ICC_FOUND-NOTFOUND")
      set (ICC_ENV_SCRIPT iccvars_${ICC_PLATFORM}.sh)
  endif()
	set (ICC_CL icpc)
	set (ICC_LD icpc)
	# check that all executables are indeed in path
  find_program (ICC_SCRIPT_FOUND ${ICC_ENV_SCRIPT} NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH NO_CMAKE_FIND_ROOT_PATH)
  if (${ICC_SCRIPT_FOUND} STREQUAL "ICC_SCRIPT_FOUND-NOTFOUND")
    message (FATAL_ERROR "ICC (${ICC_ENV_SCRIPT}) is not installed in the system")
  endif()
  set (ICC_ENV_SCRIPT ${ICC_SCRIPT_FOUND})
  find_program (ICC_CL_FOUND ${ICC_CL} NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH NO_CMAKE_FIND_ROOT_PATH)
  if (${ICC_CL_FOUND} STREQUAL "ICC_CL_FOUND-NOTFOUND")
    message (FATAL_ERROR "ICC (${ICC_CL}) is not installed in the system")
  endif()
  set (ICC_CL ${ICC_CL_FOUND})
  find_program (ICC_LD_FOUND ${ICC_LD} NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH NO_CMAKE_FIND_ROOT_PATH)
  if (${ICC_LD_FOUND} STREQUAL "ICC_LD_FOUND-NOTFOUND")
    message (FATAL_ERROR "ICC (${ICC_LD}) is not installed in the system")
  endif()
  set (ICC_LD ${ICC_LD_FOUND})
endif (WIN32)

# icc wrappers (they are required in order to provide run-time environment for spawned ICC)
if (WIN32)
	set (ICC_COMPILER_SCRIPT compile_win.bat)
	set (ICC_LINKER_SCRIPT   link_win.bat)
else()
  set (ICC_COMPILER_SCRIPT compile_linux.sh)
  set (ICC_LINKER_SCRIPT   link_linux.sh)
endif ()

message ("CLANG:     ${CLANG}")
message ("LLVM_OPT:  ${LLVM_OPT}")
message ("LLVM_LINK: ${LLVM_LINK}")
message ("TRIPLE:    ${TRIPLE}")
if (WIN32)
	message ("ICC INSTALL PATH: ${ICC_WIN32_PATH}")
endif ()

# 
# Tool environment and options
#

# clang
set (OCL_OUTPUT_EXTENSION .rtl)
set (CLANG_COMPILER_PARAMS 
							-cc1 -triple ${TRIPLE} -emit-llvm-bc -disable-free -mdisable-fp-elim
							-mconstructor-aliases	-fcolor-diagnostics -O3 -opencl-builtins
              -mrelocation-model static -fmessage-length 200 -fms-extensions
              -fgnu-runtime -fdiagnostics-show-option -fcolor-diagnostics
              -x cl -I ${IMPORT_DIR} -I ${CLANG_HEADERS} -include opencl_.h
    )

# llvm-opt
set (LLVM_OPT_PARAMS -scalarrepl -functionattrs)

# llvm-link
set (LLVM_LINK_PARAMS -f)

# icc
if (WIN32)
		list (APPEND ICC_COMPILER_PARAMS_LIST 
              /nologo /c /I ${IMPORT_DIR} /DWIN32 /D_WINDOWS /D_USRDLL 
              /DCL_BUILTIN_FUNCTIONS_EXPORTS /D_WINDLL /D_UNICODE /DUNICODE
              /EHsc /GS /fp:fast /W3 /Zi
      	 )
    set (BUILD_TYPE_ICC_COMPILER_FLAGS_Debug "/Od /D_DEBUG /RTC1 /MTd")
    set (BUILD_TYPE_ICC_COMPILER_FLAGS_Release "/O2 /Oi	/Qipo /DNDEBUG /MT /Gy")

  	list (APPEND ICC_LINKER_PARAMS_LIST 
              /nologo /INCREMENTAL:NO kernel32.lib user32.lib gdi32.lib 
              winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib
              oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /TLBID:1 
              /SUBSYSTEM:WINDOWS /DYNAMICBASE /NXCOMPAT /DLL
      	)
 
    if (CMAKE_SIZEOF_VOID_P EQUAL 4)
    	list (APPEND ICC_LINKER_PARAMS_LIST "/MACHINE:X86")
    else ()
      list (APPEND ICC_LINKER_PARAMS_LIST "/MACHINE:X64")
    endif ()
    set (BUILD_TYPE_ICC_LINKER_FLAGS_Debug "/DEBUG")
    set (BUILD_TYPE_ICC_LINKER_FLAGS_Release "/DEBUG /OPT:REF /OPT:ICF")
    
else ()
	 list (APPEND ICC_COMPILER_PARAMS_LIST
	            -c -DCL_BUILTIN_FUNCTIONS_EXPORTS -fPIC -I ${IMPORT_DIR} -U__MMX__
	      )
	 set (BUILD_TYPE_ICC_COMPILER_FLAGS_Debug "-g")
   set (BUILD_TYPE_ICC_COMPILER_FLAGS_Release "-O3")
 
	 list (APPEND ICC_LINKER_PARAMS_LIST 
	            -fPIC -static-intel -shared -Wl,-rpath=\\\$ORIGIN
	 	    )
    set (BUILD_TYPE_ICC_LINKER_FLAGS_Debug "-g")
    set (BUILD_TYPE_ICC_LINKER_FLAGS_Release "-g")
endif (WIN32)

# the same lists, but converted into single strings (otherwise they cannot be feeded to ICC compiler and linker scripts)
foreach (FLAG ${ICC_COMPILER_PARAMS_LIST})
    set (ICC_COMPILER_PARAMS "${ICC_COMPILER_PARAMS} ${FLAG}")
endforeach ()
foreach (FLAG ${ICC_LINKER_PARAMS_LIST})
    set (ICC_LINKER_PARAMS "${ICC_LINKER_PARAMS} ${FLAG}")
endforeach ()

#------------------- Utility functions ---------------------------

#
# Compile single OpenCL source file
#
# compile_ocl( OUTPUT_FILE_VAR ARCH PROC INPUT_FILE OUTPUT_DIR )
#
#   OUTPUT_FILE_VAR - output, contains full path to object file
#   ARCH            - target SIMD arch type
#   PROC            - target CPU
#   INPUT_FILE      - full path to input file
#   OUTPUT_DIR      - directory for output file
#

function( compile_ocl OUTPUT_FILE_VAR ARCH PROC INPUT_FILE OUTPUT_DIR )

    get_filename_component (NAME ${INPUT_FILE} NAME_WE )
    set (OUTPUT_FILE ${OUTPUT_DIR}/${NAME}_${ARCH}${OCL_OUTPUT_EXTENSION} )
	
	  # -- BUGBUG -- ${LLVM_OPT} dependency is removed as it is changed most frequently during LLVM development and 
	  #              in the context of BI Clang compilation these changes have least impact on generated IR
    add_custom_command (OUTPUT ${OUTPUT_FILE}
                        COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_DIR}
                        COMMAND ${CLANG} ${CLANG_COMPILER_PARAMS} -target-cpu ${PROC} -o ${OUTPUT_FILE} ${INPUT_FILE}
					              COMMAND ${LLVM_OPT} ${LLVM_OPT_PARAMS} -o ${OUTPUT_FILE} ${OUTPUT_FILE}
                        MAIN_DEPENDENCY ${INPUT_FILE}
                        DEPENDS ${CLANG} ${HEADER_FILES}
                        IMPLICIT_DEPENDS CXX ${INPUT_FILE}
                        WORKING_DIRECTORY ${CL_BUILTIN_SOURCE_DIR}
                        COMMENT "OCL Compiling ${INPUT_FILE}"
                        VERBATIM
                       )

    set (${OUTPUT_FILE_VAR} ${OUTPUT_FILE} PARENT_SCOPE )

endfunction( compile_ocl )

#
# add OpenCL library built from set of OpenCL source files
#
# add_ocl_library( OUTPUT_FILE_VAR TARGET_NAME ARCH PROC OUTPUT_DIR ...FILES... )
#
#   OUTPUT_FILE_VAR - output, contains generated library
#		TARGET_NAME     - name of the target and base name of the lib. Extension is OCL_OUTPUT_EXTENSION
#   ARCH            - target SIMD arch type
#   PROC            - target CPU
#   OUTPUT_DIR      - output directory for the intermediate files
#   FILES           - input files list
#

function( add_ocl_library OUTPUT_FILE_VAR TARGET_NAME ARCH PROC OUTPUT_DIR)

    if (${ARGC} LESS 6)
        message (FATAL_ERROR "add_ocl_library(${TARGET_NAME}) - no sources given")
    endif ()

    set (OUTPUT_FILE ${BUILTINS_INSTALL_DIR}/${TARGET_NAME}${OCL_OUTPUT_EXTENSION})

	  # Compile all files and prepare a 'TMP_FILES' list of .rtl files
	  foreach (FILE ${ARGN})
		    compile_ocl( TMP_RTL ${ARCH} ${PROC} ${FILE} ${OUTPUT_DIR})
		    list (APPEND TMP_FILES ${TMP_RTL})
	  endforeach (FILE)

    # link OpenCL library (adding also BI IR for vectorizer)
    add_custom_command (OUTPUT ${OUTPUT_FILE}
                        COMMAND ${CMAKE_COMMAND} -E make_directory ${BUILTINS_INSTALL_DIR}
                        COMMENT "OCL Linking ${TARGET_NAME}${OCL_OUTPUT_EXTENSION}"
                        COMMAND ${LLVM_LINK} ${LLVM_LINK_PARAMS} -o ${OUTPUT_FILE} ${TMP_FILES} ${CL_BUILTIN_SOURCE_DIR}/${TARGET_NAME}_vectorizer.bc
                        DEPENDS ${LLVM_LINK} ${TMP_FILES} ${CL_BUILTIN_SOURCE_DIR}/${TARGET_NAME}_vectorizer.bc
                        VERBATIM
                       )

 		set (${OUTPUT_FILE_VAR} ${OUTPUT_FILE} PARENT_SCOPE)
 
endfunction( add_ocl_library )

#
# Compile single C source file
#
# compile_c( OUTPUT_FILE_VAR ARCH CPP_DEFINE AUX_COMPILE_OPT INPUT_FILE OUTPUT_DIR )
#
#   OUTPUT_FILE_VAR - output, contains full path to object file
#   ARCH            - target SIMD arch type
#   CPP_DEFINE      - arch-dependent #define for compilation
#   AUX_COMPILE_OPT - additional compilation params
#   INPUT_FILE      - full path to input file
#   OUTPUT_DIR      - directory for output file
#

function( compile_c OUTPUT_FILE_VAR ARCH CPP_DEFINE AUX_COMPILE_OPT INPUT_FILE OUTPUT_DIR )

    get_filename_component (NAME ${INPUT_FILE} NAME_WE )
    set (OUTPUT_FILE ${OUTPUT_DIR}/${NAME}_${ARCH}${CMAKE_C_OUTPUT_EXTENSION} )
	
    add_custom_command (OUTPUT ${OUTPUT_FILE}
                        COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_DIR}
                        COMMAND ${ICC_COMPILER_SCRIPT} ${CL_BUILD_TYPE} ${INPUT_FILE} ${OUTPUT_FILE} ${CPP_DEFINE} ${AUX_COMPILE_OPT} 
                        MAIN_DEPENDENCY ${INPUT_FILE}
                        DEPENDS ${ICC_COMPILER_SCRIPT} ${HEADER_FILES}
                        IMPLICIT_DEPENDS CXX ${INPUT_FILE}
                        WORKING_DIRECTORY ${CL_BUILTIN_SOURCE_DIR}
                        COMMENT "C Compiling ${INPUT_FILE}"
                        VERBATIM
                       )

    set (${OUTPUT_FILE_VAR} ${OUTPUT_FILE} PARENT_SCOPE )

endfunction( compile_c )

#
# add C library built from set of C source files
#
# add_c_library( OUTPUT_FILE_VAR TARGET_NAME ARCH CPP_DEFINE AUX_COMPILE_OPT OUTPUT_DIR ...FILES... )
#
#   OUTPUT_FILE_VAR - output, contains generated library
#   TARGET_NAME     - name of the target and base name of the lib. Extension is CMAKE_SHARED_LIBRARY_SUFFIX
#   ARCH            - target SIMD arch type
#   CPP_DEFINE      - arch-dependent #define for compilation
#   AUX_COMPILE_OPT - additional compilation params
#   OUTPUT_DIR      - output directory for the intermediate files
#   FILES           - input files list
#

function( add_c_library OUTPUT_FILE_VAR TARGET_NAME ARCH CPP_DEFINE AUX_COMPILE_OPT OUTPUT_DIR)

    if (${ARGC} LESS 7)
        message (FATAL_ERROR "add_c_library(${TARGET_NAME}) - no sources given")
    endif ()

    set (OUTPUT_FILE ${BUILTINS_INSTALL_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${TARGET_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX})

    # SVML import library (for Win32 only - hence the presence check is delegated to Win32 link script)
    set (SVML_IMPORT_LIB ${SVML_NAME_PREFIX}${ARCH}${CMAKE_IMPORT_LIBRARY_SUFFIX})

    # PDB file will be generated for Win32
    set (PDB_FILE ${BUILTINS_INSTALL_DIR}/${TARGET_NAME}.pdb)

		# SVML binary
	  set (SVML_DLL ${SVML_DLL_DIR}/${SVML_NAME_PREFIX}${ARCH}${CMAKE_SHARED_LIBRARY_SUFFIX})
		if (NOT EXISTS ${SVML_DLL})
		   message( FATAL_ERROR "Cannot find ${SVML_DLL}!")
		endif()

	  # Compile all files and prepare a 'TMP_FILES' list of object files
	  foreach (FILE ${ARGN})
		    compile_c( TMP_OBJ ${ARCH} ${CPP_DEFINE} ${AUX_COMPILE_OPT} ${FILE} ${OUTPUT_DIR})
		    set (TMP_FILES "${TMP_FILES} ${TMP_OBJ}")
		    list (APPEND TMP_FILES_DEPEND ${TMP_OBJ})
	  endforeach (FILE)

    # link C library (together with SVML import lib - for Win32 only)
    add_custom_command (OUTPUT ${OUTPUT_FILE}
                        COMMAND ${CMAKE_COMMAND} -E make_directory   ${BUILTINS_INSTALL_DIR}
                        COMMAND ${CMAKE_COMMAND} -E copy ${SVML_DLL} ${BUILTINS_INSTALL_DIR}/
                        COMMAND ${ICC_LINKER_SCRIPT} ${CL_BUILD_TYPE} ${OUTPUT_FILE} ${SVML_IMPORT_LIB} ${PDB_FILE} ${TMP_FILES}
                        DEPENDS ${ICC_LINKER_SCRIPT} ${TMP_FILES_DEPEND} ${RESOURCE_FILE}
                        COMMENT "C Linking ${CMAKE_SHARED_LIBRARY_PREFIX}${TARGET_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}"
                        VERBATIM
                       )

		set (${OUTPUT_FILE_VAR} ${OUTPUT_FILE} PARENT_SCOPE)
		
endfunction( add_c_library )

#
# create_target( PROC ARCH CPP_DEFINE AUX_COMPILE_OPT)
#
# Uses:
#   TARGET_NAME_PREFIX - base name of the target DLL and OpenCL library
#   C_SOURCE_FILES     - list of sources for DLL
#   OCL_SOURCE_FILES   - list of sources for OpenCL library
# Parameters
#   PROC            - target CPU
#   ARCH            - target SIMD arch type
#   CPP_DEFINE      - arch-dependent #define for compilation
#   AUX_COMPILE_OPT - additional compilation params

function( create_target PROC ARCH CPP_DEFINE AUX_COMPILE_OPT )

	set (TARGET_NAME ${TARGET_NAME_PREFIX}${ARCH})

	message ("Building target:   ${TARGET_NAME}")

	# intermediate directory for OpenCL and ICC compilation/link
	# note that platform is already accounted by ${CMAKE_CURRENT_BINARY_DIR}
	set (INTERMEDIATE_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/${INSTALL_SUBDIR})

  # build .rtl file out of OCL code (by clang) and copy it to install directory
  add_ocl_library( OUTPUT_OCL_LIB ${TARGET_NAME} ${ARCH} ${PROC} ${INTERMEDIATE_OUTPUT_DIR} ${OCL_SOURCE_FILES})

  # build DLL/.so file out of C code (by ICC) and copy it to install directory
  add_c_library( OUTPUT_C_LIB ${TARGET_NAME} ${ARCH} ${CPP_DEFINE} ${AUX_COMPILE_OPT} ${INTERMEDIATE_OUTPUT_DIR} ${C_SOURCE_FILES})

  # top-level target for both OCL and C targets
  add_custom_target (${TARGET_NAME} ALL
                     DEPENDS ${OUTPUT_OCL_LIB} ${OUTPUT_C_LIB}
                     COMMENT "Target ${TARGET_NAME} build completed"
                     VERBATIM
                     SOURCES ${OCL_SOURCE_FILES} ${C_SOURCE_FILES} ${HEADER_FILES} ${RESOURCE_FILE}
                    )

	if (DEFINED OCL_SOURCE_DIR)
		set_target_properties (${TARGET_NAME} PROPERTIES FOLDER "LLVM/cl_builtins")
	endif ()

endfunction( create_target)

#------------------ Main -----------------------

# OpenCL source code
list (APPEND OCL_SOURCE_FILES
        atomic_functions.cpp
        common_functions.cpp
        conversion_functions.cpp
        geometry_functions.cpp
        image_functions.cpp
        integer_functions.cpp
        math_functions.cpp
        misc_functions.cpp
        relational_functions.cpp
        vload_vstore_functions.cpp
     )

# C source code
list (APPEND C_SOURCE_FILES
        conversion_functions.cpp
        image_functions.cpp            
     )
# resource file
set (RESOURCE_FILE ${CL_BUILTIN_SOURCE_DIR}/clbltfn.rc)

# project header files
list (APPEND HEADER_FILES
				cl_atomic_declaration.h
				cl_common_declaration.h
				cl_conversions_declaration.h
				cl_geom_declaration.h
				cl_image_declaration.h
				cl_integer_declaration.h
				cl_math_declaration.h
				cl_relational_declaration.h
				cl_vloadvstore_declaration.h
				conversions_workaround.h
				stdafx.h
        targetver.h
     )

# convert everything to full paths
set (SAVE_OCL_SOURCE_FILES ${OCL_SOURCE_FILES})
unset (OCL_SOURCE_FILES)
foreach (FILE ${SAVE_OCL_SOURCE_FILES})
    list (APPEND OCL_SOURCE_FILES ${CL_BUILTIN_SOURCE_DIR}/${FILE})
endforeach (FILE)

set (SAVE_C_SOURCE_FILES ${C_SOURCE_FILES})
unset (C_SOURCE_FILES)
foreach (FILE ${SAVE_C_SOURCE_FILES})
    list (APPEND C_SOURCE_FILES ${CL_BUILTIN_SOURCE_DIR}/${FILE})
endforeach (FILE)

set (SAVE_HEADER_FILES ${HEADER_FILES})
unset (HEADER_FILES)
foreach (FILE ${SAVE_HEADER_FILES})
    list (APPEND HEADER_FILES ${CL_BUILTIN_SOURCE_DIR}/${FILE})
endforeach (FILE)

# conversion of filepaths used during configuration of ICC compiler and linker scripts to native format (as they will be directly used by these scripts)
file (TO_NATIVE_PATH ${ICC_ENV_SCRIPT} ICC_ENV_SCRIPT_NATIVE)
file (TO_NATIVE_PATH ${ICC_CL}         ICC_CL_NATIVE)
file (TO_NATIVE_PATH ${ICC_LD}         ICC_LD_NATIVE)
file (TO_NATIVE_PATH ${SVML_LIB_DIR}   SVML_LIB_DIR_NATIVE)
file (TO_NATIVE_PATH ${RESOURCE_FILE}  RESOURCE_FILE_NATIVE)

# configuring ICC compiler and linker scripts (the configured scripts are in ${CMAKE_CURRENT_BINARY_DIR})
configure_file (${ICC_COMPILER_SCRIPT} ${CMAKE_CURRENT_BINARY_DIR}/${ICC_COMPILER_SCRIPT} @ONLY)
configure_file (${ICC_LINKER_SCRIPT}   ${CMAKE_CURRENT_BINARY_DIR}/${ICC_LINKER_SCRIPT} @ONLY)   

# redefine ICC compiler and linker scripts to their new locations
set (ICC_COMPILER_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/${ICC_COMPILER_SCRIPT})
set (ICC_LINKER_SCRIPT   ${CMAKE_CURRENT_BINARY_DIR}/${ICC_LINKER_SCRIPT})

# running build for all supported SIMD architectures 
add_subdirectory (core2)
add_subdirectory (corei7)
add_subdirectory (penryn)
add_subdirectory (prescott)
add_subdirectory (sandybridge)
