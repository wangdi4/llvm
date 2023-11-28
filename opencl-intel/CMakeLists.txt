#
# INTEL CONFIDENTIAL
#
# Copyright (C) 2022 Intel Corporation
#
# This software and the related documents are Intel copyrighted materials, and
# your use of them is governed by the express license under which they were
# provided to you ("License"). Unless the License provides otherwise, you may
# not use, modify, copy, publish, distribute, disclose or transmit this software
# or the related documents without Intel's prior written permission.
#
# This software and the related documents are provided as is, with no express or
# implied warranties, other than those that are expressly stated in the License.
#
# Global setup file for OpenCL CMake
#
cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0001 NEW)
cmake_policy(SET CMP0011 OLD)
cmake_policy(SET CMP0054 OLD)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(OPENCL_INTREE_BUILD OFF)
  project(OCL C CXX ASM)
else()
  set(OPENCL_INTREE_BUILD ON)
endif()

# This is a workaround for the following CMake bug:
# https://gitlab.kitware.com/cmake/cmake/-/issues/21462
# A fix was merged and will seemingly be in 3.24+.
set(CMAKE_ASM_COMPILE_OPTIONS_MSVC_RUNTIME_LIBRARY_MultiThreaded         "")
set(CMAKE_ASM_COMPILE_OPTIONS_MSVC_RUNTIME_LIBRARY_MultiThreadedDLL      "")
set(CMAKE_ASM_COMPILE_OPTIONS_MSVC_RUNTIME_LIBRARY_MultiThreadedDebug    "")
set(CMAKE_ASM_COMPILE_OPTIONS_MSVC_RUNTIME_LIBRARY_MultiThreadedDebugDLL "")

# Project-wise options
option(GIT_REVISIONS "Make package version from git revision" ON)
option(USE_ITT_INTERNAL "Report internal tasks start/stop to VTune" OFF)
option(USE_HARD_TRAPPING "Use hard trapping for workers on CPU device" ON)
option(BUILD_LLVM_FROM_SOURCE "Build the llvm package from source instead of
                               using pre-installed binaries" OFF)
option(LLVM_BUILD_OCL_CPU_BACKEND "Build OpenCL CPU backend DLL." ON)
option(USE_ITT "Enable ITT support" ON)

# validation
# TODO: add 'master' option to disable all kinds of validation
option(BACKEND_BUILD_VERIFICATION_LIB "Build back-end validation tools." ON)
option(ADD_NICE_BE_LIT_TARGETS "Generate cmake targets for launching BE LIT
  tests from each subdirectory separately" ON)
option(OPENCL_BE_INCLUDE_TESTS "Generate build targets for OpenCL BE unittest.
  Defaults to ON. You can use this option to disable the generation of build
  targets for OpenCL BE LIT tests." ON)
option(OPENCL_RT_INCLUDE_TESTS "Generate build targets for OpenCL RT unittest.
  Defaults to ON. You can use this option to disable the generation of build
  targets for OpenCL RT unittests." ON)
option(OPENCL_RT_BUILD_TESTS "Build OpenCL RT unittests. Defaults to OFF.
  Targets for building each unittest are generated in any case." OFF)

set(OCL_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(OCL_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

# Set path to the CMAKE modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

include(FetchContent)
include(AddOpenCLClang)

if(NOT OPENCL_ASM_COMPILER)
  # OPENCL_ASM_COMPILER defines the assembler program to build assembly sources.
  # If it is not defined, then use the default.
  set(OPENCL_ASM_COMPILER ${CMAKE_ASM_COMPILER})
endif()

include(CheckCXXCompilerFlag)

# If in-tree we don't need to include HandleLLVMOptions again: it contains logic
# which should not be evaluted twice.
if(NOT OPENCL_INTREE_BUILD)
  if(NOT DEFINED LLVM_PATH_FE OR NOT DEFINED LLVM_PATH_BE)
    message(FATAL_ERROR "LLVM_PATH_FE or LLVM_PATH_BE is not specified.")
  endif()
  set(LLVM_PATH ${LLVM_PATH_FE})
  find_package(LLVM REQUIRED)

  include(${LLVM_PATH}/lib/cmake/llvm/AddLLVM.cmake)
  include(HandleLLVMOptions)
endif()

# Include functions for opencl targets
include(CMakeFuncs)

use_eh(TRUE)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(BUILD_X64 True)
endif()

include(AddOpenCL)
if(WIN32)
  include(CMake_windows_tools)
else()
  include(CMake_linux_tools)
endif()

# INTEL_CUSTOMIZATION
# Common defines
if(INTEL_PRODUCT_RELEASE)
  add_definitions(-DINTEL_PRODUCT_RELEASE)
endif(INTEL_PRODUCT_RELEASE)
# end INTEL_CUSTOMIZATION

if((NOT WIN32) AND USE_HARD_TRAPPING)
  add_definitions(-D__HARD_TRAPPING__)
endif((NOT WIN32) AND USE_HARD_TRAPPING)

if(BUILD_OPENCL_21)
  add_definitions(-DBUILD_OPENCL_21)
endif(BUILD_OPENCL_21)

# Add OpenCL 3.0 definitions
add_definitions(-DCL_TARGET_OPENCL_VERSION=300)

add_definitions(
  -DTBB_BINARIES_POSTFIX="${TBB_BINARIES_POSTFIX}"
  -D__TBB_NO_IMPLICIT_LINKAGE=1 -DCL_USE_DEPRECATED_OPENCL_1_1_APIS)

# define INTEL_CUSTOMIZATION xmain macro
# TODO: consider to remove from OpenCL CMake scripts
add_definitions(-DINTEL_CUSTOMIZATION)

# Enable ITT
if(USE_ITT)
  add_definitions(-DUSE_ITT)
  include_directories(${OCL_SOURCE_DIR}/externals/itt/include)
endif()

if(WIN32 AND NOT OPENCL_INTREE_BUILD)
  # Add definitions that make MSVC much less annoying.
  # FIXME: all these flags are added to RC_FLAGS, but most of them do not make
  # much sense for RC compiler. These have to be added as flags, not as
  # definitions.
  add_definitions(
    # For some reason MS wants to deprecate a bunch of standard functions...
    -D_CRT_SECURE_NO_DEPRECATE
    -D_CRT_SECURE_NO_WARNINGS
    -D_CRT_NONSTDC_NO_DEPRECATE
    -D_CRT_NONSTDC_NO_WARNINGS
    -D_SCL_SECURE_NO_DEPRECATE
    -D_SCL_SECURE_NO_WARNINGS
    -wd4141 # Suppress ''modifier' : used more than once' (because of
            # __forceinline combined with inline)
    -wd4065 # Suppress 'switch statement contains 'default' but no 'case'
            # labels'
    -wd4090 # Suppress 'function : different 'const' qualifiers'
    -wd4146 # Suppress 'unary minus operator applied to unsigned type, result
            # still unsigned'
    -wd4180 # Suppress 'qualifier applied to function type has no meaning;
            # ignored'
    -wd4190 # Suppress 'function has C linkage is specified, but returns type
            # which is incompatible with C'
    -wd4224 # Suppress 'nonstandard extension used : formal parameter
            # 'identifier' was previously defined as a type'
    -wd4244 # Suppress ''argument' : conversion from 'type1' to 'type2',
            # possible loss of data'
    -wd4267 # Suppress ''var' : conversion from 'size_t' to 'type', possible
            # loss of data'
    -wd4275 # Suppress 'An exported class was derived from a class that was not
            # exported.'
    -wd4291 # Suppress ''declaration' : no matching operator delete found;
            # memory will not be freed if initialization throws an exception'
    -wd4345 # Suppress 'behavior change: an object of POD type constructed with
            # an initializer of the form () will be default-initialized'
    -wd4351 # Suppress 'new behavior: elements of array 'array' will be default
            # initialized'
    -wd4355 # Suppress ''this' : used in base member initializer list'
    -wd4503 # Suppress ''identifier' : decorated name length exceeded, name was
            # truncated'
    -wd4551 # Suppress 'function call missing argument list'
    -wd4624 # Suppress ''derived class' : destructor could not be generated
            # because a base class destructor is inaccessible'
    -wd4715 # Suppress ''function' : not all control paths return a value'
    -wd4800 # Suppress ''type' : forcing value to bool 'true' or 'false'
            # (performance warning)'
    -wd4065 # Suppress 'switch statement contains 'default' but no 'case'
            # labels'
    -wd4181 # Suppress 'qualifier applied to reference type; ignored'
    -w14062 # Promote "enumerator in switch of enum is not handled" to level 1
            # warning.
    -wd4985 # Supress 'MSVS9 Math.h "ceil": attributes not present on previous
            # declaration'
  )
endif()

# See llvm community commit c64108430f43dc98b1c8aa9d45215003e7a6fc58
#
# Turn off missing field initializer warnings for gcc to avoid noise from false
# positives with empty {}. Turn them on otherwise (they're off by default for
# clang).
if(CMAKE_COMPILER_IS_GNUCXX)
  add_flag_if_supported("-Wno-missing-field-initializers"
                        CMPLR_NO_MISSING_FIELD_INITIALIZERS)
else()
  add_flag_if_supported("-Wmissing-field-initializers"
                        CMPLR_MISSING_FIELD_INITIALIZERS)
endif()

if(NOT OPENCL_INTREE_BUILD)
  # Supported configurations
  set(CMAKE_CONFIGURATION_TYPES "Debug" "Release")
  set(CMAKE_CONFIGURATION_TYPES
      ${CMAKE_CONFIGURATION_TYPES}
      CACHE STRING "Available build configurations." FORCE)
endif()

if(NOT WIN32)
  add_custom_target(
    strip
    ${OCL_SOURCE_DIR}/cmake/separate_linux_symbols.sh
    WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/bin
    COMMENT "Separating debug symbols from binaries...")
endif(NOT WIN32)

if(BUILD_X64)
  set(ADDR 64)
else()
  set(ADDR 32)
endif()

# set windows binary suffix
if(WIN32)
  set(BUILD_PLATFORM ${ADDR})
else(WIN32)
  set(BUILD_PLATFORM "")
endif(WIN32)

if(NOT OPENCL_INTREE_BUILD)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()
set(CMAKE_C_STANDARD 99)

if(OPENCL_INTREE_BUILD)
  set(CLANG_OPENCL_HEADERS_DIR ${LLVM_EXTERNAL_CLANG_SOURCE_DIR}/lib/Headers)

  set(LLVM_INCLUDE_DIRS ${LLVM_MAIN_INCLUDE_DIR})
  set(LLVM_PATH_BE ${LLVM_BINARY_DIR})
  set(LLVM_PATH_FE ${LLVM_BINARY_DIR})
endif()

find_package(Threads REQUIRED)
if(NOT CMAKE_THREAD_LIBS_INIT)
  if(PTHREAD_LIB STREQUAL "")
    set(PTHREAD_LIB "$ENV{PTHREAD_LIB}")
  endif()
  if(PTHREAD_LIB STREQUAL "")
    message(
      FATAL_ERROR
        "No pthread library found!\nRun the cmake with option -D PTHREAD_LIB=/path/to/library or set the enviroment variable PTHREAD_LIB")
  endif()
else(NOT CMAKE_THREAD_LIBS_INIT)
  set(PTHREAD_LIB ${CMAKE_THREAD_LIBS_INIT})
endif()

set(REVISION_ID $ENV{REVISION_ID})
set(GIT_VERSION $ENV{REVISION})
set(GIT_REVISION $ENV{RELEASE_BRANCH})

if(NOT REVISION_ID)
  message(STATUS "Build type: Development")
  set(BUILDVER_WC_REVISION "0")
  set(GIT_VERSION "DEV")
  set(GIT_REVISION "development")
elseif(REVISION_ID MATCHES "\\.[0-9]*") # Revision ID
  message(STATUS "Build type: Release")
  string(REGEX MATCH ".([0-9]*)" REV_ID ${REVISION_ID})
  set(BUILDVER_WC_REVISION ${CMAKE_MATCH_1})
  set(GIT_VERSION $ENV{REVISION})
  set(GIT_REVISION $ENV{RELEASE_BRANCH})
elseif(REVISION_ID MATCHES "-r[0-9]*") # Review ID
  message(STATUS "Build type: Review")
  set(BUILDVER_WC_REVISION "0")
  set(GIT_VERSION $ENV{REVISION})
  set(GIT_REVISION $ENV{RELEASE_BRANCH})
endif()

# Align OpenCL version to source code revision if revision is not "head"
if("${OCL_REVISION}" MATCHES "^[0-9]+(_[0-9]+)?")
  # Get year value
  string(SUBSTRING "${OCL_REVISION}" 0 4 PRODUCTVER_MAJOR)
  # Get month value
  string(SUBSTRING "${OCL_REVISION}" 4 2 PRODUCTVER_MINOR)
  # If revision is "head", we use build time
else("${OCL_REVISION}" MATCHES "^[0-9]+(_[0-9]+)?")
  string(TIMESTAMP PRODUCTVER_MAJOR "%Y")
  string(TIMESTAMP PRODUCTVER_MINOR "%m")
endif()

# Make month number single digit
string(FIND ${PRODUCTVER_MINOR} "0" ZERO_POS)
if(${ZERO_POS} STREQUAL "0")
  string(SUBSTRING ${PRODUCTVER_MINOR} 1 1 PRODUCTVER_MINOR)
endif(${ZERO_POS} STREQUAL "0")
set(PRODUCTVER_MINOR "${PRODUCTVER_MINOR}.0")

math(EXPR BUILDVER_16_BIT_MAJOR "${BUILDVER_WC_REVISION} / 65536")
if(DEFINED ENV{BUILDDATE})
  set(BUILDVER_16_BIT_MINOR "$ENV{BUILDDATE}")
else()
  string(TIMESTAMP MONTH "%m")
  string(TIMESTAMP DAY "%d")
  set(BUILDVER_16_BIT_MINOR ${MONTH}${DAY})
endif()
message("BUILDVER_16_BIT_MAJOR: ${BUILDVER_16_BIT_MAJOR}")
message("BUILDVER_16_BIT_MINOR: ${BUILDVER_16_BIT_MINOR}")

message(STATUS "Current build version is ${BUILDVER_WC_REVISION}")
message(STATUS "GIT Version: ${GIT_VERSION}")
message(STATUS "GIT Revision: ${GIT_REVISION}")
message(STATUS "PRODUCTVER_MAJOR: ${PRODUCTVER_MAJOR}")

# Define product info (currently uses only for win resource file generation)
set(PRODUCTNAME "Intel(R) OpenCL")
set(COMPANYNAME "Intel Corporation")
set(LEGALCOPYRIGHT "Copyright (C) 2020-2021 ${COMPANYNAME}. All rights reserved.")

# --------------
# write a file with the BUILDVERSION define
file(
  WRITE ${OCL_BINARY_DIR}/buildversion.h.txt
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
#define IOCVERSIONSTRING \"1.6.${BUILDVER_16_BIT_MAJOR}.${BUILDVER_16_BIT_MINOR}\"\n
#define KBVERSIONSTRING \"1.4.${BUILDVER_16_BIT_MAJOR}.${BUILDVER_16_BIT_MINOR}\"\n"
)
# --------------
# copy the file to the final header only if the version changes reduces needless
# rebuilds
execute_process(
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${OCL_BINARY_DIR}/buildversion.h.txt ${OCL_BINARY_DIR}/buildversion.h)

if(WIN32)
  file(
    WRITE ${OCL_BINARY_DIR}/version.ini
    "[Version]\nPackVer=${PRODUCTVER_MAJOR}.${PRODUCTVER_MINOR}.${BUILDVER_WC_REVISION}\n"
  )
  install_to(${OCL_BINARY_DIR}/version.ini DESTINATION ./ COMPONENT ocl-version)
endif()
file(
  WRITE ${OCL_BINARY_DIR}/version.txt
  "Version=${PRODUCTVER_MAJOR}.${PRODUCTVER_MINOR}.${BUILDVER_16_BIT_MAJOR}.${BUILDVER_16_BIT_MINOR}\nInternalVersion=${GIT_REVISION}\n"
)
file(
  WRITE ${OCL_BINARY_DIR}/revision.txt
  "BUILD=${GIT_VERSION}\nREVISION=${GIT_REVISION}\nLLVM_REVISION_FE=${LLVM_REVISION_FE}\nLLVM_REVISION_BE=${LLVM_REVISION_BE}\nLLVM_PATH_BE=${LLVM_PATH_BE}\nLLVM_PATH_FE=${LLVM_PATH_FE}\n"
)
install_to(${OCL_BINARY_DIR}/revision.txt DESTINATION ./ COMPONENT ocl-version)

# copy the version file to install folder
install_to(${OCL_BINARY_DIR}/version.txt DESTINATION ./ COMPONENT ocl-version)

# Define OpenCL libraries version
set_opencl_version()

# Prepare llvm libraries which are needed by ocl
llvm_map_components_to_libnames(
  LLVM_MODULE_LIBS
  AllTargetsAsmParsers
  AllTargetsCodeGens
  AllTargetsDescs
  AllTargetsDisassemblers
  AllTargetsInfos
  Analysis
  BitReader
  BitWriter
  CodeGen
  Core
  ExecutionEngine
  IPO
  IRReader
  InstCombine
  IntelJITEvents
  Interpreter
  Linker
  MCJIT
  Option
  OrcJIT
  Passes
  SPIRVLib
  ScalarOpts
  Support
  Symbolize
  Target
  TransformUtils
  SYCLTransforms
  )

# Set opencl-clang library properties that rely on above configurations.
set_opencl_clang_extra_properties()

if(WIN32 AND ((${CMAKE_CFG_INTDIR} STREQUAL $(OutDir))
              OR (${CMAKE_CFG_INTDIR} STREQUAL $(Configuration))))
  set(INSTALL_SUBDIR ${CMAKE_CFG_INTDIR})
else()
  set(INSTALL_SUBDIR ${CMAKE_BUILD_TYPE})
endif()

set(OCL_GLOBAL_INCLUDE ${OCL_SOURCE_DIR}/cl_api)

# set OS-dependent source tree elements
if(WIN32)
  set(OS Win)
  set(IMPLIB_SUBDIR lib)
  set(IMPLIB_PREFIX)
  set(IMPLIB_SUFFIX .lib)
  set(IMPLIB_STATIC_SUFFIX .lib)
  set(DLL_SUFIX .dll)
  set(DYNAMIC_PREFIX dyn_)
else()
  set(OS Lin)
  set(IMPLIB_SUBDIR bin)
  set(IMPLIB_PREFIX lib)
  set(IMPLIB_SUFFIX .so)
  set(IMPLIB_STATIC_SUFFIX .a)
  set(DLL_SUFIX .so)
  set(DYNAMIC_PREFIX "")
endif()

set(OCL_SYS_DEPENDENT_SUBDIR ${OS}${ADDR}_${INSTALL_SUBDIR})
set(OCL_SYS_DEPENDENT_IMPLIB_SUBDIR
    ${IMPLIB_SUBDIR}/${OCL_SYS_DEPENDENT_SUBDIR})
# END OF set OS-dependent source tree elements

set_unicode_off()

# TBB library and include path
set(TBB_ROOT_DIR $ENV{TBBROOT}/$ENV{TBBVER})
message(STATUS "Environment TBBROOT: $ENV{TBBROOT}")
message(STATUS "Environment TBBVER: $ENV{TBBVER}")

# The new version of TBBConfig.cmake file has a bug when determining
# whether it's a 32bit environment. This is a W/A to fix this issue.
if(NOT BUILD_X64)
  set(_tbb_arch_suffix 32)
endif()

# TBB_LIBRARY_DIR - Used for LIT environment
if(EXISTS "${TBB_ROOT_DIR}/tbb")
  set(TBB_DIR "${TBB_ROOT_DIR}/tbb/latest/lib/cmake/tbb")
else()
  set(TBB_DIR "${TBB_ROOT_DIR}/lib/cmake/tbb")
endif()
find_package(TBB REQUIRED tbb)
get_target_property(TBB_LOC TBB::tbb LOCATION_RELEASE)
get_target_property(TBB_INC_DIR TBB::tbb INTERFACE_INCLUDE_DIRECTORIES)
get_filename_component(TBB_LIBRARY_DIR ${TBB_LOC} DIRECTORY)

# INTEL_CUSTOMIZATION
# On windows, we are building xmain in the u4win environment. And msvc utils
# just like link.exe cannot recognize rdrive path. So we need to copy tbb imp
# libs to local directory.
if(WIN32)
  get_target_property(TBB_IMPLIB_RELEASE TBB::tbb IMPORTED_IMPLIB_RELEASE)
  get_target_property(TBB_IMPLIB_DEBUG TBB::tbb IMPORTED_IMPLIB_DEBUG)
  execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory
                    ${OCL_BINARY_DIR}/lib/${OUTPUT_ARCH_SUFF})
  execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${TBB_IMPLIB_RELEASE} ${OCL_BINARY_DIR}/lib/${OUTPUT_ARCH_SUFF}
                  COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${TBB_IMPLIB_DEBUG} ${OCL_BINARY_DIR}/lib/${OUTPUT_ARCH_SUFF})
  link_directories(${OCL_BINARY_DIR}/lib/${OUTPUT_ARCH_SUFF})
endif(WIN32)
# end INTEL_CUSTOMIZATION

if((NOT TBB_LIBRARY_DIR) OR (NOT EXISTS ${TBB_LIBRARY_DIR}))
  message(FATAL_ERROR "TBB library path is not found: ${TBB_LIBRARY_DIR}")
endif()

if((NOT TBB_INC_DIR) OR (NOT EXISTS ${TBB_INC_DIR}))
  message(FATAL_ERROR "TBB include path is not found: ${TBB_INC_DIR}")
endif()

set(TBB_INCLUDE_DIRS ${TBB_INC_DIR} utils/task_executor/enhancements)

# paths
add_llvm_external_project(opencl)
set(CL_API_HEADERS ${OpenCL_INCLUDE_DIR} ${OCL_SOURCE_DIR}/framework/inc)
# OPENCL_ICD_LOADER_HEADERS_DIR is defined in opencl/CMakeLists.txt
set(CL_LOADER ${OPENCL_ICD_LOADER_HEADERS_DIR}/../ocl-icd-src/)

get_property(SPIRV_HEADERS_DIR GLOBAL PROPERTY spirv_headers_dir_property)
include_directories(
  BEFORE
  utils/cl_logger/export
  utils/cl_hw_utils/export
  utils/cl_sys_utils/export
  devices/device_commands/export
  ${SPIRV_HEADERS_DIR}/include
  ${TBB_INCLUDE_DIRS})

# Framework IDE Folders Names
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(FRAMEWORK_FOLDER_NAME Framework)
set(API_FOLDER_NAME ${FRAMEWORK_FOLDER_NAME}/API)
set(DEVICES_FOLDER_NAME ${FRAMEWORK_FOLDER_NAME}/Devices)
set(RUNTIME_FOLDER_NAME ${FRAMEWORK_FOLDER_NAME}/Runtime)
set(UTILS_FOLDER_NAME ${FRAMEWORK_FOLDER_NAME}/utils)
set(FRONTEND_FOLDER_NAME Frontend)
set(EXTERNAL_FOLDER_NAME External)

if(BUILD_LLVM_FROM_SOURCE)
  add_subdirectory(${LLVM_SRC_ROOT} "${CMAKE_CURRENT_BINARY_DIR}/llvm")
endif()

# FindLLVM.cmake will overwrite LLVM_BINARY_DIR, so we save it as LLVM_OBJ_DIR.
set(LLVM_OBJ_DIR ${LLVM_BINARY_DIR})

# Use CLANG_VERSION_* if it's specified, otherwise use LLVM_VERSION_*.
if(NOT DEFINED CLANG_VERSION_MAJOR)
  set(CLANG_VERSION_MAJOR ${LLVM_VERSION_MAJOR})
endif()
if(NOT DEFINED CLANG_VERSION_MINOR)
  set(CLANG_VERSION_MINOR ${LLVM_VERSION_MINOR})
endif()
if(NOT DEFINED CLANG_VERSION_PATCHLEVEL)
  set(CLANG_VERSION_PATCHLEVEL ${LLVM_VERSION_PATCH})
endif()
# Unlike PACKAGE_VERSION, CLANG_VERSION does not include LLVM_VERSION_SUFFIX.
set(CLANG_VERSION
    "${CLANG_VERSION_MAJOR}.${CLANG_VERSION_MINOR}.${CLANG_VERSION_PATCHLEVEL}")

# Used by code coverage tests to find clang profile lib
set(CLANG_LIBRARY_DIR
    "${LLVM_LIBRARY_DIR}/clang/${CLANG_VERSION_MAJOR}/lib/${LLVM_TARGET_TRIPLE}")

# Let's help CMake to find llvm-lit
if(NOT OPENCL_INTREE_BUILD)
  set(LLVM_EXTERNAL_LIT "${LLVM_PATH_BE}/bin/llvm-lit")
  if(WIN32 AND NOT CYGWIN)
    set(LLVM_EXTERNAL_LIT "${LLVM_EXTERNAL_LIT}.py")
  endif(WIN32 AND NOT CYGWIN)
endif()

# The conformance tests must be compiled with the headers located in
# ${OCL_CONFORMANCE_INCLUDE} and not the ones in ${OCL_GLOBAL_INCLUDE}.
# Therefore, it must be added to the include directories only after the tests
# subdirectory has been added.
include_directories(BEFORE ${OCL_GLOBAL_INCLUDE})

add_subdirectory(externals)

if(OPENCL_BE_INCLUDE_TESTS OR OPENCL_RT_INCLUDE_TESTS)
  # Download opencl cpp headers for tests
  if(INTEL_CUSTOMIZATION)
    if(DEFINED ENV{ICS_GIT_MIRROR} AND NOT "$ENV{ICS_GIT_MIRROR}" STREQUAL "")
      STRING(REGEX REPLACE "\\\\" "/" GITSERVER "$ENV{ICS_GIT_MIRROR}")
    else()
      set(GITSERVER "https://github.com/intel-innersource")
    endif()
    set(OPENCL_CPP_HEADER_REPO "${GITSERVER}/applications.compilers.source.opencl-clhpp.git")
  endif(INTEL_CUSTOMIZATION)

  set(OPENCL_CPP_HEADER_TAG 4a1157466afe72a87e8abc59537ef577534ccadf)

  message(STATUS "Will fetch opencl c++ header from ${OPENCL_CPP_HEADER_REPO}")
  FetchContent_Declare(
    opencl-cpp-header
    GIT_REPOSITORY ${OPENCL_CPP_HEADER_REPO}
    GIT_TAG ${OPENCL_CPP_HEADER_TAG}
    GIT_SUBMODULES "docs"
  )
  FetchContent_Populate(opencl-cpp-header)

  if (NOT EXISTS ${opencl-cpp-header_SOURCE_DIR})
    message(FATAL_ERROR "opencl cpp header is not fetched from ${OPENCL_CPP_HEADER_REPO}")
  endif()
  # An umbrella target for all tests inside current folder.
  umbrella_lit_testsuite_begin(check-ocl-all)
endif()

# Compiler
add_subdirectory(FrontendDriver)

# Versioning for OclCpuDebugging DLL.
set(DEBUGGING_DLL_VERSION 9)
set(DEBUGGING_DLL_NAME OclCpuDebugging)
add_definitions(-DDEBUGGING_DLL_VERSION=${DEBUGGING_DLL_VERSION})
add_definitions(-DDEBUGGING_DLL_NAME=${DEBUGGING_DLL_NAME})

add_subdirectory(backend)

# Framework
add_subdirectory(framework)
add_subdirectory(devices)
add_subdirectory(cl_api)
add_subdirectory(utils)

if(OPENCL_RT_INCLUDE_TESTS)
  add_subdirectory(tests)
endif(OPENCL_RT_INCLUDE_TESTS)

if(OPENCL_BE_INCLUDE_TESTS OR OPENCL_RT_INCLUDE_TESTS)
  umbrella_lit_testsuite_end(check-ocl-all)
  set_target_properties(check-ocl-all PROPERTIES FOLDER "Tests")
endif()

# Entry target for ocl cpu compilation.
add_custom_target(opencl-cpu-rt
  COMMENT "Compile all OpenCL CPU RT libraries."
  DEPENDS process-intrin-headers common_clang${BUILD_PLATFORM} intelocl${BUILD_PLATFORM} cllibrary OpenCL-ICD
)

# Entry target for fpga emulator compilation
add_custom_target(opencl-fpga-emu
  COMMENT "Compile all OpenCL FPGA emulator libraries."
  DEPENDS process-intrin-headers common_clang${BUILD_PLATFORM} intelocl${BUILD_PLATFORM}_${OUTPUT_EMU_SUFF} cllibrary OpenCL-ICD
)

set(INTEL_DEPLOY_COMPONENTS_SHARED
    OpenCL-ICD
    ocl-library-kernel
    ocl-builtins
    ocl-builtins-shared
    ocl-common_clang
    ocl-config
    ocl-gdb-plugin
    ocl-svml
    ocl-tbb)

if(WIN32)
  set(INTEL_DEPLOY_COMPONENTS_SHARED
      ${INTEL_DEPLOY_COMPONENTS_SHARED}
      ocl-${DEBUGGING_DLL_NAME}${DEBUGGING_DLL_VERSION})
endif(WIN32)

set(INTEL_DEPLOY_COMPONENTS_CPU_RT
    ocl-intelocl${BUILD_PLATFORM}
    ${INTEL_DEPLOY_COMPONENTS_SHARED})

set(INTEL_DEPLOY_COMPONENTS_FPGA_EMU
    ocl-intelocl${BUILD_PLATFORM}_${OUTPUT_EMU_SUFF}
    ${INTEL_DEPLOY_COMPONENTS_SHARED})

# This command is used for creating a fake dependency in order to force
# another command(s) to execute. Its output is never produced as a result thus
# each other command that depends on it will always be executed. We use this
# to force running deploy installation regardless of manifest files state.
add_custom_command(OUTPUT __force_it COMMAND "${CMAKE_COMMAND}" -E echo)

# Deploy OpenCL CPU RT
deploy_ocl_components(deploy-ocl-cpu "${INTEL_DEPLOY_COMPONENTS_CPU_RT}" opencl-cpu-rt)
# Deploy FPGA Emu
deploy_ocl_components(deploy-fpga-emu "${INTEL_DEPLOY_COMPONENTS_FPGA_EMU}" opencl-fpga-emu )
# This target is added for not intree build. For example, win32 compilation.
# INTEL_CUSTOMIZATION
if(NOT OPENCL_INTREE_BUILD)
  deploy_ocl_components(deploy "${INTEL_DEPLOY_COMPONENTS_CPU_RT}" opencl-cpu-rt)
endif(NOT OPENCL_INTREE_BUILD)
# end INTEL_CUSTOMIZATION

if(NOT DEFINED INTEL_DEPLOY_COMPONENTS_CPU_RT AND NOT DEFINED INTEL_DEPLOY_COMPONENTS_FPGA_EMU)
  # no specific components was given so do full installation
  set(manifest ${CMAKE_CURRENT_BINARY_DIR}/install_manifest.txt)
  add_custom_command(
    OUTPUT ${manifest}
    COMMAND "${CMAKE_COMMAND}" -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
    DEPENDS __force_it
    COMMENT "Performing deploy operation..."
    USES_TERMINAL)
  add_custom_target(deploy DEPENDS ${manifest})
endif()
