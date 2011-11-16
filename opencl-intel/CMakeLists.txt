#
# Global setup file for OpenCL CMake
#
cmake_minimum_required(VERSION 2.8.4)
cmake_policy(SET CMP0001 NEW)
cmake_policy(SET CMP0011 OLD)

include( cmake_utils/CMakeFuncs.txt )


# the FindSubversion.cmake module is part of the standard distribution
include(FindSubversion)

string(REPLACE " " ";" CONFORMANCE_LIST ${CONFORMANCE_LIST})

if (WIN32)
    include( cmake_utils/CMake_windows_tools.cmake )
else ()
    include( cmake_utils/CMake_linux_tools.cmake )
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

# extract working copy information for all components into variables
Subversion_WC_INFO(${OCL_SOURCE_DIR} SDKVER)
Subversion_WC_INFO(${OCL_SOURCE_DIR}/backend/ocl_cpu_backend BEVER)

message("Current build version is ${SDKVER_WC_REVISION}.${BEVER_WC_REVISION}")

# vars
set( OCL_MAKE_UTILS_DIR ${OCL_SOURCE_DIR}/cmake_utils )

if (WIN32 AND ${CMAKE_CFG_INTDIR} STREQUAL $(OutDir))
    set( INSTALL_SUBDIR ${CMAKE_CFG_INTDIR} )
else ()
    set( INSTALL_SUBDIR ${CMAKE_BUILD_TYPE} )
endif (WIN32 AND ${CMAKE_CFG_INTDIR} STREQUAL $(OutDir))

set( OCL_GLOBAL_INCLUDE   ${OCL_SOURCE_DIR}/cl_api )
set( OCL_CLANG_HEADERS   ${OCL_SOURCE_DIR}/backend/clang_headers )

# write a file with the BUILDVERSION define
file(WRITE ${OCL_BINARY_DIR}/buildversion.h.txt "#define BUILDVERSION ${SDKVER_WC_REVISION}.${BEVER_WC_REVISION}\n#define BUILDVERSIONSTR "\"(Build\ ${SDKVER_WC_REVISION}.${BEVER_WC_REVISION})\")
file(WRITE ${OCL_BINARY_DIR}/version.txt "3.0.${SDKVER_WC_REVISION}.${BEVER_WC_REVISION}")

# copy the file to the final header only if the version changes
# reduces needless rebuilds
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different ${OCL_BINARY_DIR}/buildversion.h.txt ${OCL_BINARY_DIR}/buildversion.h)

# paths
include_directories( BEFORE
        utils/cl_logger/export
        utils/cl_sys_utils/export
        externals/tbb/include
        ${OCL_GLOBAL_INCLUDE}
    )

if (WIN32 AND NOT( "$ENV{DXSDK_DIR}" STREQUAL "") )
	message ("Compiling with DX sharing")
	add_definitions (-DDX9_MEDIA_SHARING)
endif ()

# set OS-dependent source tree elements
if (WIN32)
    set(OS            Win)
    set(IMPLIB_SUBDIR lib)
    set(IMPLIB_PREFIX    )
    set(IMPLIB_SUFFIX .lib)
	set(DLL_SUFIX .dll)
else ()
    set(OS Lin)
    set(IMPLIB_SUBDIR bin)
    set(IMPLIB_PREFIX lib)
    set(IMPLIB_SUFFIX .so)
	set(DLL_SUFIX .so)
endif( WIN32)

if (CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(ADDR 32)
else ()
    set(ADDR 64)
endif (CMAKE_SIZEOF_VOID_P EQUAL 4)

set(OCL_SYS_DEPENDENT_SUBDIR        ${OS}${ADDR}_${INSTALL_SUBDIR})
set(OCL_SYS_DEPENDENT_IMPLIB_SUBDIR ${IMPLIB_SUBDIR}/${OCL_SYS_DEPENDENT_SUBDIR})

# END OF set OS-dependent source tree elements

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

# externals
add_subdirectory(externals)

add_subdirectory( cl_api )
add_subdirectory( utils )

# Backend + LLVM + Clang
add_subdirectory( backend )

#devices
option(INCLUDE_MIC_DEVICE "Include MIC device Into Build"  OFF)
add_subdirectory( devices )

#FE compilers
add_subdirectory( fe_compilers )

# Framework
add_subdirectory( framework )

# Common Runime
option(INCLUDE_CMRT "Include Common Runtime into Build"  OFF)
if (WIN32 AND INCLUDE_CMRT)
	message ("Adding Common Runtime sources...")
	add_subdirectory ( common_runtime )
endif(WIN32 AND INCLUDE_CMRT)
#tools
add_subdirectory ( tools )

#validation
add_subdirectory( validation/types )
