#
# Global setup file for OpenCL CMake
#
cmake_minimum_required(VERSION 2.6.3)
cmake_policy(SET CMP0001 NEW)
cmake_policy(SET CMP0011 OLD)

include( cmake_utils/CMakeFuncs.txt )

#
string(REPLACE " " ";" CONFORMANCE_LIST ${CONFORMANCE_LIST})

if (WIN32)
    include( cmake_utils/CMake_windows_tools.txt )
else ()
    include( cmake_utils/CMake_linux_tools.txt )
endif (WIN32)

#PRINT_TOOLS()

#
# Project defs
#

project(OCL)
#
# Project variables:
#  OCL_SOURCE_DIR    - points to the top src directory
#  OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root of generated files
#

# vars
set( OCL_MAKE_UTILS_DIR ${OCL_SOURCE_DIR}/cmake_utils )

if (WIN32 AND ${CMAKE_CFG_INTDIR} STREQUAL $(OutDir))
    set( INSTALL_SUBDIR ${CMAKE_CFG_INTDIR} )
else ()
    set( INSTALL_SUBDIR ${CMAKE_BUILD_TYPE} )
endif (WIN32 AND ${CMAKE_CFG_INTDIR} STREQUAL $(OutDir))

set( OCL_INSTALL_DIR      ${OCL_BINARY_DIR}/__bin/bin.${BIN_OUTPUT_DIR_SUFFIX}/${INSTALL_SUBDIR} )
set( OCL_GLOBAL_INCLUDE   ${OCL_SOURCE_DIR}/cl_api )


# paths
include_directories( BEFORE
        utils/cl_logger/export
        utils/cl_sys_utils/export
        externals/tbb/include
        ${OCL_GLOBAL_INCLUDE}
    )

# set OS-dependent source tree elements
if (WIN32)
    set(OS            Win)
    set(IMPLIB_SUBDIR lib)
    set(IMPLIB_PREFIX    )
    set(IMPLIB_SUFFIX .lib)
else ()
    set(OS Lin)
    set(IMPLIB_SUBDIR bin)
    set(IMPLIB_PREFIX lib)
    set(IMPLIB_SUFFIX .so)
endif( WIN32)

if (CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(ADDR 32)
else ()
    set(ADDR 64)
endif (CMAKE_SIZEOF_VOID_P EQUAL 4)

set(OCL_SYS_DEPENDENT_SUBDIR        ${OS}${ADDR}_${INSTALL_SUBDIR})
set(OCL_SYS_DEPENDENT_IMPLIB_SUBDIR ${IMPLIB_SUBDIR}/${OCL_SYS_DEPENDENT_SUBDIR})

# END OF set OS-dependent source tree elements

#link_directories( ${OCL_SOURCE_DIR}/externals/tbb/${OCL_SYS_DEPENDENT_IMPLIB_SUBDIR}
#		  ${OCL_SOURCE_DIR}/externals/llvm_wrap/${OCL_SYS_DEPENDENT_IMPLIB_SUBDIR}	)

SET_UNICODE_ON()

set_property( GLOBAL PROPERTY USE_FOLDERS ON )

# Framework IDE Folders Names
set( FRAMEWORK_FOLDER_NAME      Framework )
set( API_FOLDER_NAME            ${FRAMEWORK_FOLDER_NAME}/API )
set( DEVICES_FOLDER_NAME        ${FRAMEWORK_FOLDER_NAME}/Devices )
set( RUNTIME_FOLDER_NAME        ${FRAMEWORK_FOLDER_NAME}/Runtime )
set( UTILS_FOLDER_NAME          ${FRAMEWORK_FOLDER_NAME}/utils )
set( TOOLS_FOLDER_NAME          ${FRAMEWORK_FOLDER_NAME}/Tools )
set( FE_COMPILER_FOLDER_NAME    "${FRAMEWORK_FOLDER_NAME}/Front-end Compiler" )
set( EXTERNAL_FOLDER_NAME       External )

#recurse

# Framework
add_subdirectory( framework )
add_subdirectory( cl_api )
add_subdirectory( utils )
add_subdirectory( externals/tbb )
add_subdirectory( externals/tal )

# Backend

add_subdirectory( externals/llvm_wrap )

#devices
add_subdirectory( devices )
add_subdirectory( fe_compilers/clang_compiler )

#tools
add_subdirectory( tools/OfflineCompiler )
add_subdirectory( tools/OpenCLTracer )

#gtest
add_subdirectory( externals/gtest )

#validation
add_subdirectory( validation/types/workloads_test_type )
add_subdirectory( validation/types/cpu_device_test_type )
add_subdirectory( validation/types/framework_test_type )


option(INCLUDE_CONFORMANCE_TESTS "Include Conformance Tests Into Build"  OFF)

if (INCLUDE_CONFORMANCE_TESTS)

if (WIN32)	
	
	set(MSVC 1)
	set(USE_ICC 1)
	add_subdirectory( validation/types/khronos_conformance )
	
else (WIN32)
	add_subdirectory( validation/types/khronos_conformance )
endif(WIN32)
    
endif (INCLUDE_CONFORMANCE_TESTS)

