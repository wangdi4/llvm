#
# Global setup file for OpenCL CMake
#
cmake_minimum_required(VERSION 2.8.7)
cmake_policy(SET CMP0001 NEW)
cmake_policy(SET CMP0011 OLD)
cmake_policy(SET CMP0054 OLD)

if( BUILD_EXPERIMENTAL_21 )
    set( OPENCL_BINARIES_POSTFIX "_2_1" )
    set( TBB_BINARIES_POSTFIX "_2_1" )

    add_definitions( -DBUILD_EXPERIMENTAL_21 )
endif( BUILD_EXPERIMENTAL_21 )

if( BUILD_FPGA_EMULATOR )
    set( OPENCL_BINARIES_POSTFIX "_emu" )

    add_definitions( -DBUILD_FPGA_EMULATOR  )
endif( BUILD_FPGA_EMULATOR )

add_definitions( -DOPENCL_BINARIES_POSTFIX="${OPENCL_BINARIES_POSTFIX}" )
add_definitions( -DTBB_BINARIES_POSTFIX="${TBB_BINARIES_POSTFIX}" )

# CMAKE variables
set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    "${CMAKE_SOURCE_DIR}/cmake/Modules/"
    "${CMAKE_SOURCE_DIR}/cmake_utils/"
)
include( cmake_utils/CMakeFuncs.txt )

if (WIN32)
  include( CMake_windows_tools )
else()
  include( CMake_linux_tools )
endif ()

if( INTEL_PRODUCT_RELEASE )
  add_definitions( -DINTEL_PRODUCT_RELEASE )
endif( INTEL_PRODUCT_RELEASE )

# Supported configurations
set(CMAKE_CONFIGURATION_TYPES
    "Debug"
    "Release"
  )
set(CMAKE_CONFIGURATION_TYPES ${CMAKE_CONFIGURATION_TYPES} CACHE STRING "Available build configurations." FORCE)


# ITT/VTune integration
add_definitions( -DUSE_ITT )
include_directories( ${CMAKE_SOURCE_DIR}/externals/itt/include
                     ${CMAKE_SOURCE_DIR}/externals/itt/ittnotify/ )

if (NOT WIN32)
  # enable C++11 support in GCC 4.7 and above
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

  add_custom_target(strip
    ${CMAKE_SOURCE_DIR}/cmake_utils/separate_linux_symbols.sh
    WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/bin
    COMMENT "Separating debug symbols from binaries...")
endif (NOT WIN32)

if (WIN32)
    # Add definitions that make MSVC much less annoying.
    add_definitions(
        # For some reason MS wants to deprecate a bunch of standard functions...
        -D_CRT_SECURE_NO_DEPRECATE
        -D_CRT_SECURE_NO_WARNINGS
        -D_CRT_NONSTDC_NO_DEPRECATE
        -D_CRT_NONSTDC_NO_WARNINGS
        -D_SCL_SECURE_NO_DEPRECATE
        -D_SCL_SECURE_NO_WARNINGS

        -wd4141 # Suppress ''modifier' : used more than once' (because of __forceinline combined with inline)
        -wd4065 # Suppress 'switch statement contains 'default' but no 'case' labels'
        -wd4090 # Suppress 'function : different 'const' qualifiers'
        -wd4146 # Suppress 'unary minus operator applied to unsigned type, result still unsigned'
        -wd4180 # Suppress 'qualifier applied to function type has no meaning; ignored'
        -wd4190 # Suppress 'function has C linkage is specified, but returns type which is incompatible with C'
        -wd4224 # Suppress 'nonstandard extension used : formal parameter 'identifier' was previously defined as a type'
        -wd4244 # Suppress ''argument' : conversion from 'type1' to 'type2', possible loss of data'
        -wd4267 # Suppress ''var' : conversion from 'size_t' to 'type', possible loss of data'
        -wd4275 # Suppress 'An exported class was derived from a class that was not exported.'
        -wd4291 # Suppress ''declaration' : no matching operator delete found; memory will not be freed if initialization throws an exception'
        -wd4345 # Suppress 'behavior change: an object of POD type constructed with an initializer of the form () will be default-initialized'
        -wd4351 # Suppress 'new behavior: elements of array 'array' will be default initialized'
        -wd4355 # Suppress ''this' : used in base member initializer list'
        -wd4503 # Suppress ''identifier' : decorated name length exceeded, name was truncated'
        -wd4551 # Suppress 'function call missing argument list'
        -wd4624 # Suppress ''derived class' : destructor could not be generated because a base class destructor is inaccessible'
        -wd4715 # Suppress ''function' : not all control paths return a value'
        -wd4800 # Suppress ''type' : forcing value to bool 'true' or 'false' (performance warning)'
        -wd4065 # Suppress 'switch statement contains 'default' but no 'case' labels'
        -wd4181 # Suppress 'qualifier applied to reference type; ignored'
        -w14062 # Promote "enumerator in switch of enum is not handled" to level 1 warning.
        -wd4985 # Supress 'MSVS9 Math.h "ceil": attributes not present on previous declaration'
    )

    #C flags
    set( CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG}   /MDd")
    set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MD")

    # C++ flags
    set( CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   /MDd")
    set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
endif (WIN32)

if(CMAKE_CROSSCOMPILING)
    set(CX_NATIVE_TG_DIR "${CMAKE_BINARY_DIR}/native")

    add_custom_command(OUTPUT ${CX_NATIVE_TG_DIR}
                            COMMAND ${CMAKE_COMMAND} -E make_directory ${CX_NATIVE_TG_DIR}
                            COMMENT "Creating ${CX_NATIVE_TG_DIR}..."
    )

    add_custom_command(OUTPUT ${CX_NATIVE_TG_DIR}/CMakeCache.txt
                            COMMAND ${CMAKE_COMMAND} -UCMAKE_TOOLCHAIN_FILE
                            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                            # We are cross compiling when building for android, hence we are
                            # dropping off all non required build components, such tests/verification
                            -DBACKEND_BUILD_VERIFICATION_LIB=OFF
                            -DLLVM_PATH_BE=${LLVM_PATH_BE}/native
                            -DLLVM_PATH_FE=${LLVM_PATH_FE}/native
                            -DBUILD_X64=True
                            ${CMAKE_SOURCE_DIR}
                            WORKING_DIRECTORY ${CX_NATIVE_TG_DIR}
                            DEPENDS ${CX_NATIVE_TG_DIR}
                            COMMENT "Configuring native targets..."
    )

    # Build 'native' tools required to build target binaries:
    # ocloptimg - required to build OpenCL built-in library
    # CMake can build one target at a time and we need only ocloptimg, so we
    # just use it. If you need to build several targets, you should create
    # custom target with dependencies from all the target you need to build.
    # This will allow you to build them in parallel!
    # NOTE: actually we cheat here, we also need table-gen, but ocloptimg has
    # it in its depedencies, so we just check that it's built with ocloptimg.
    add_custom_command(OUTPUT ${CX_NATIVE_TG_DIR}/bin/ocloptimg ${CX_NATIVE_TG_DIR}/bin/ocl-tblgen
                            COMMAND ${CMAKE_COMMAND}
                            --build ${CX_NATIVE_TG_DIR}
                            --target ocloptimg
                            --config ${CMAKE_BUILD_TYPE} -- -j
                            WORKING_DIRECTORY ${CX_NATIVE_TG_DIR}
                            DEPENDS ${CX_NATIVE_TG_DIR}/CMakeCache.txt
                            COMMENT "Building native targets..."
    )

    add_custom_target(ConfigureNativeTargets DEPENDS ${CX_NATIVE_TG_DIR}/bin/ocloptimg ${CX_NATIVE_TG_DIR}/bin/ocl-tblgen )
    set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${CX_NATIVE_TG_DIR})
else(CMAKE_CROSSCOMPILING)
    add_custom_target(ConfigureNativeTargets DEPENDS ${CMAKE_BINARY_DIR}/bin/ocloptimg ${CMAKE_BINARY_DIR}/bin/ocl-tblgen )
endif(CMAKE_CROSSCOMPILING)

# Project-wise options
option(GIT_REVISIONS "Make package version from git revision"  ON)
option(BUILD_QTGUI "Build the QT related projects"  OFF)
option(USE_ITT_INTERNAL "Report internal tasks start/stop to VTune" OFF)
option(USE_HARD_TRAPPING "Use hard trapping for workers on CPU device"  ON)
option(INCLUDE_CMRT "Include Common Runtime into Build"  OFF)
option(BUILD_LLVM_FROM_SOURCE "Build the llvm package from source instead of using pre-installed binaries" OFF)

#
# Project defs
#
project(OCL C CXX)

FIND_PACKAGE ( Threads REQUIRED )
if(NOT CMAKE_THREAD_LIBS_INIT)
  if(PTHREAD_LIB STREQUAL "")
    set(PTHREAD_LIB "$ENV{PTHREAD_LIB}")
  endif()
  if(PTHREAD_LIB STREQUAL "")
    message(FATAL_ERROR "No pthread library found!\nRun the cmake with option -D PTHREAD_LIB=/path/to/library or set the enviroment variable PTHREAD_LIB")
  endif()
else(NOT CMAKE_THREAD_LIBS_INIT)
  set(PTHREAD_LIB ${CMAKE_THREAD_LIBS_INIT})
endif()

set( REVISION_ID $ENV{REVISION_ID} )
set( GIT_VERSION $ENV{REVISION} )
set( GIT_REVISION $ENV{RELEASE_BRANCH} )

if(NOT REVISION_ID)
  message(STATUS "Build type: Development")
  set(BUILDVER_WC_REVISION "0")
  set( GIT_VERSION "DEV" )
  set( GIT_REVISION "development" )
elseif ( REVISION_ID MATCHES "\\.[0-9]*" ) # Revision ID
  message(STATUS "Build type: Release")
  STRING(REGEX MATCH ".([0-9]*)" REV_ID ${REVISION_ID} )
  set(BUILDVER_WC_REVISION ${CMAKE_MATCH_1})
  set( GIT_VERSION $ENV{REVISION} )
  set( GIT_REVISION $ENV{RELEASE_BRANCH} )
elseif ( REVISION_ID MATCHES "-r[0-9]*" ) # Review ID
  message(STATUS "Build type: Review")
  set( BUILDVER_WC_REVISION "0" )
  set( GIT_VERSION $ENV{REVISION} )
  set( GIT_REVISION $ENV{RELEASE_BRANCH} )
endif()

# MAJOR and MINOR version aligned with 15.40 branch because currently
# we are delivering cpu bundle from master.
# As soon as we will deliver cpu buncle from release branch to vpg
# revert this change. CSSD100021694.
set( PRODUCTVER_MAJOR 7 )
set( PRODUCTVER_MINOR 6 )

math(EXPR BUILDVER_16_BIT_MAJOR "${BUILDVER_WC_REVISION} / 65536")
math(EXPR BUILDVER_16_BIT_MINOR "${BUILDVER_WC_REVISION} % 65536")

message(STATUS "Current build version is ${BUILDVER_WC_REVISION}")
message(STATUS "GIT Version: ${GIT_VERSION}")
message(STATUS "GIT Revision: ${GIT_REVISION}")
message(STATUS "PRODUCTVER_MAJOR: ${PRODUCTVER_MAJOR}")

# --------------
#  write a file with the BUILDVERSION define
file(WRITE ${OCL_BINARY_DIR}/buildversion.h.txt
"#define BUILDVERSION ${BUILDVER_WC_REVISION}\n
#ifndef\ _DEBUG\n
    #define BUILDVERSIONSTR \"(Build\ ${BUILDVER_WC_REVISION})\"\n
#else\n
    #define BUILDVERSIONSTR \"(Build[DEBUG]\ ${BUILDVER_WC_REVISION})\"\n
#endif\n
#define PRODUCTVERSION_MAJOR ${PRODUCTVER_MAJOR}\n
#define PRODUCTVERSION_MINOR ${PRODUCTVER_MINOR}\n
#define BUILDVERSION16BIT_MAJOR ${BUILDVER_16_BIT_MAJOR}\n
#define BUILDVERSION16BIT_MINOR ${BUILDVER_16_BIT_MINOR}\n
#define VERSIONSTRING \"${PRODUCTVER_MAJOR}.${PRODUCTVER_MINOR}.${BUILDVER_16_BIT_MAJOR}.${BUILDVER_16_BIT_MINOR}\"\n
#define IOCVERSIONSTRING \"1.6.${BUILDVER_16_BIT_MAJOR}.${BUILDVER_16_BIT_MINOR}\"\n
#define KBVERSIONSTRING \"1.4.${BUILDVER_16_BIT_MAJOR}.${BUILDVER_16_BIT_MINOR}\"\n")
# --------------

if(WIN32)
    file(WRITE ${OCL_BINARY_DIR}/version.ini "[Version]\nPackVer=${PRODUCTVER_MAJOR}.${PRODUCTVER_MINOR}.${BUILDVER_WC_REVISION}\n")
    install(FILES ${OCL_BINARY_DIR}/version.ini DESTINATION ./)
endif()
file(WRITE ${OCL_BINARY_DIR}/version.txt "Version=${PRODUCTVER_MAJOR}.${PRODUCTVER_MINOR}.${BUILDVER_16_BIT_MAJOR}.${BUILDVER_16_BIT_MINOR}\nInternalVersion=${GIT_REVISION}\n")
file(WRITE ${OCL_BINARY_DIR}/revision.txt "BUILD=${GIT_VERSION}\nREVISION=${GIT_REVISION}\nLLVM_REVISION_FE=${LLVM_REVISION_FE}\nLLVM_REVISION_BE=${LLVM_REVISION_BE}\nLLVM_PATH_BE=${LLVM_PATH_BE}\nLLVM_PATH_FE=${LLVM_PATH_FE}\n")
install(FILES ${OCL_BINARY_DIR}/revision.txt DESTINATION ./)

#copy the version file to install folder
install(FILES ${OCL_BINARY_DIR}/version.txt DESTINATION ./)
# copy the file to the final header only if the version changes
# reduces needless rebuilds
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different ${OCL_BINARY_DIR}/buildversion.h.txt ${OCL_BINARY_DIR}/buildversion.h)

# vars

if(CONFORMANCE_LIST)
    string(REPLACE " " ";" CONFORMANCE_LIST ${CONFORMANCE_LIST})
endif()

add_definitions (-DCL_USE_DEPRECATED_OPENCL_1_1_APIS)

if (WIN32 AND ((${CMAKE_CFG_INTDIR} STREQUAL $(OutDir)) OR (${CMAKE_CFG_INTDIR} STREQUAL $(Configuration))))
    set( INSTALL_SUBDIR ${CMAKE_CFG_INTDIR} )
else ()
    set( INSTALL_SUBDIR ${CMAKE_BUILD_TYPE} )
endif ()

set( OCL_GLOBAL_INCLUDE  ${OCL_SOURCE_DIR}/cl_api )
set( OCL_CLANG_HEADERS   ${OCL_SOURCE_DIR}/backend/clang_headers )

# set OS-dependent source tree elements
if (WIN32)
    set(OS            Win)
    set(IMPLIB_SUBDIR lib)
    set(IMPLIB_PREFIX    )
    set(IMPLIB_SUFFIX .lib)
    set(IMPLIB_STATIC_SUFFIX .lib)
    set(DLL_SUFIX .dll)
    set(DYNAMIC_PREFIX dyn_ )
else ()
    set(OS Lin)
    set(IMPLIB_SUBDIR bin)
    set(IMPLIB_PREFIX lib)
    set(IMPLIB_SUFFIX .so)
    set(IMPLIB_STATIC_SUFFIX .a)
    set(DLL_SUFIX .so)
    set(DYNAMIC_PREFIX "" )
endif( )

if (CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(ADDR 32)
else ()
    set(ADDR 64)
endif (CMAKE_SIZEOF_VOID_P EQUAL 4)

# set windows binary suffix
if (WIN32)
    set (BUILD_PLATFORM ${ADDR})
else (WIN32)
    set (BUILD_PLATFORM "")
endif (WIN32)

set(OCL_SYS_DEPENDENT_SUBDIR        ${OS}${ADDR}_${INSTALL_SUBDIR})
set(OCL_SYS_DEPENDENT_IMPLIB_SUBDIR ${IMPLIB_SUBDIR}/${OCL_SYS_DEPENDENT_SUBDIR})

if (WIN32)
    set( TBB_LIB_DIR ${OCL_SOURCE_DIR}/externals/tbb/${IMPLIB_SUBDIR}/VS2010${TBB_BINARIES_POSTFIX}/${OCL_SYS_DEPENDENT_SUBDIR} )
else (WIN32)
    set( TBB_LIB_DIR ${OCL_SOURCE_DIR}/externals/tbb/${OCL_SYS_DEPENDENT_IMPLIB_SUBDIR} )
endif (WIN32)
# END OF set OS-dependent source tree elements

# Define the single binary directory where the Backend tools will be placed
set(OCL_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(OCL_TOOLS_BINARY_DIR ${OCL_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OCL_TOOLS_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OCL_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OCL_BINARY_DIR}/lib)
set(OCL_RUNTIME_BINARY_DIR ${OCL_BINARY_DIR}/bin)
set(OCL_RUNTIME_LIBS_DIR  ${OCL_BINARY_DIR}/lib)
set(OCL_TESTS_BINARY_DIR ${OCL_BINARY_DIR}/tests)

SET_UNICODE_OFF()

# paths

set( TBB_INCLUDE_DIRS externals/tbb/include externals/tbb/enhancements )

include_directories( BEFORE
        utils/cl_logger/export
        utils/cl_hw_utils/export
        utils/cl_sys_utils/export
        devices/device_commands/export
        ${TBB_INCLUDE_DIRS}
    )

# Framework IDE Folders Names
set_property( GLOBAL PROPERTY USE_FOLDERS ON )
set( FRAMEWORK_FOLDER_NAME      Framework )
set( CL_API_HEADERS             ${CMAKE_SOURCE_DIR}/framework/icd/inc)
set( API_FOLDER_NAME            ${FRAMEWORK_FOLDER_NAME}/API )
set( DEVICES_FOLDER_NAME        ${FRAMEWORK_FOLDER_NAME}/Devices )
set( RUNTIME_FOLDER_NAME        ${FRAMEWORK_FOLDER_NAME}/Runtime )
set( UTILS_FOLDER_NAME          ${FRAMEWORK_FOLDER_NAME}/utils )
set( FRONTEND_FOLDER_NAME       Frontend )
set( EXTERNAL_FOLDER_NAME       External )

if( (NOT WIN32) AND USE_HARD_TRAPPING )
    add_definitions(-D__HARD_TRAPPING__)
endif((NOT WIN32) AND USE_HARD_TRAPPING)

if(BUILD_LLVM_FROM_SOURCE)
    add_subdirectory(${LLVM_SRC_ROOT} "${CMAKE_CURRENT_BINARY_DIR}/llvm")
endif()

# define INTEL_CUSTOMIZATION xmain macro
add_definitions( -DINTEL_CUSTOMIZATION )

# validation
# TODO: add 'master' option to disable all kinds of validation
option( OPENCL_RT_INCLUDE_TESTS
    "Generate build targets for OpenCL RT unittest. Defaults to ON.
    You can use this option to disable the generation of build targets
    for OpenCL RT unittests." ON )
option( OPENCL_RT_BUILD_TESTS "Build OpenCL RT unittests. Defaults to OFF.
    Targets for building each unittest are generated in any case." OFF )
if (OPENCL_RT_INCLUDE_TESTS)
    add_subdirectory( tests )
endif (OPENCL_RT_INCLUDE_TESTS)

# The conformance tests must be compiled with the headers located in
# ${OCL_CONFORMANCE_INCLUDE} and not the ones in ${OCL_GLOBAL_INCLUDE}.
# Therefore, it must be added to the include directories only after
# the tests subdirectory has been added.
include_directories( BEFORE ${OCL_GLOBAL_INCLUDE} )

add_definitions(-D__TBB_NO_IMPLICIT_LINKAGE=1)


# externals
add_subdirectory( externals )

add_subdirectory( cl_api )
add_subdirectory( utils )

# Backend
add_subdirectory( backend )

#devices
add_subdirectory( devices )

# Front-end compiler
add_subdirectory( clang_compiler )

# Framework
add_subdirectory( framework )

# Common Runtime
if ( INCLUDE_CMRT )
    add_subdirectory ( common_runtime )
endif( INCLUDE_CMRT )

#Linux samples
option(INCLUDE_SMPLS "Include Linux Samples into Build"  OFF)
if (INCLUDE_SMPLS AND NOT WIN32)
    add_subdirectory( samples )
endif(INCLUDE_SMPLS AND NOT WIN32)
