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
cmake_minimum_required(VERSION 3.4.3)
cmake_policy(SET CMP0001 NEW)
cmake_policy(SET CMP0011 OLD)
cmake_policy(SET CMP0054 OLD)

# Project-wise options
option(GIT_REVISIONS "Make package version from git revision" ON)
option(BUILD_QTGUI "Build the QT related projects" OFF)
option(USE_ITT_INTERNAL "Report internal tasks start/stop to VTune" OFF)
option(USE_HARD_TRAPPING "Use hard trapping for workers on CPU device" ON)
option(INCLUDE_CMRT "Include Common Runtime into Build" OFF)
option(BUILD_LLVM_FROM_SOURCE "Build the llvm package from source instead of
                               using pre-installed binaries" OFF)
option(LLVM_BUILD_OCL_CPU_BACKEND "Build OpenCL CPU backend DLL." ON)
option(USE_GPA "Enable GPA support" OFF)

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

# Set path to the CMAKE modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

if(NOT OPENCL_ASM_COMPILER)
  # OPENCL_ASM_COMPILER defines the assembler program to build assembly sources.
  # If it is not defined, then use the default.
  set(OPENCL_ASM_COMPILER ${CMAKE_ASM_COMPILER})
endif()

# Due to some reason, CMAKE_SIZEOF_VOID_P is empty until enable_language is
# called
enable_language(C)
enable_language(CXX)
if(NOT WIN32)
  enable_language(ASM)
endif(NOT WIN32)

include(CheckCXXCompilerFlag)

# If in-tree we don't need to include HandleLLVMOptions again: it contains logic
# which should not be evaluted twice.
if(NOT OPENCL_INTREE_BUILD)
  include(HandleLLVMOptions)
endif()

# Include functions for opencl targets
include(CMakeFuncs)

use_eh(TRUE)

include(AddOpenCL)
if(WIN32)
  include(CMake_windows_tools)
else()
  include(CMake_linux_tools)
endif()

# Common defines
if(INTEL_PRODUCT_RELEASE)
  add_definitions(-DINTEL_PRODUCT_RELEASE)
endif(INTEL_PRODUCT_RELEASE)

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

# See llvm commit adc4af6a5b70c31110559bdeb2f3f52646600b90 Author: Edwin Vane
# <edwin.vane@intel.com> Date:   Thu Jan 31 18:05:54 2013 +0000
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

#
# Project defs
#
project(OCL C CXX)

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
  set(ADDR 32)
else()
  set(ADDR 64)
endif(CMAKE_SIZEOF_VOID_P EQUAL 4)

# set windows binary suffix
if(WIN32)
  set(BUILD_PLATFORM ${ADDR})
else(WIN32)
  set(BUILD_PLATFORM "")
endif(WIN32)

set_property(GLOBAL PROPERTY CXX_STANDARD 14)
set_property(GLOBAL PROPERTY C_STANDARD 99)

set(COMMON_CLANG common_clang${BUILD_PLATFORM})
if(OPENCL_INTREE_BUILD)
  set(CLANG_OPENCL_HEADERS_DIR ${LLVM_EXTERNAL_CLANG_SOURCE_DIR}/lib/Headers)

  get_target_property(COMMON_CLANG_H_LOC ${COMMON_CLANG} SOURCE_DIR)
  if((NOT COMMON_CLANG_H_LOC) OR (NOT EXISTS
                                  "${COMMON_CLANG_H_LOC}/common_clang.h"))
    message(FATAL_ERROR "Cannot find common_clang.h from common_clang project (in ${COMMON_CLANG_H_LOC}).")
  endif()
  copy_to(${COMMON_CLANG_H_LOC}/common_clang.h DESTINATION include/cclang)
  set(CCLANG_DEV_INCLUDE_DIRS ${OCL_BINARY_DIR}/include/cclang)
  set(LLVM_INCLUDE_DIRS ${LLVM_MAIN_INCLUDE_DIR})
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
set(BUILDVER_16_BIT_MINOR "$ENV{BUILDDATE}")
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

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
  set(TBB_ARCH "ia32")
else()
  set(TBB_ARCH "intel64")
endif()

# TBB_LIBRARY_DIR - Used for LIT environment
if(WIN32)
  set(TBB_LIB_DIR ${TBB_ROOT_DIR}/lib/${TBB_ARCH}/vc14)
  set(TBB_LIBRARY_DIR ${TBB_ROOT_DIR}/redist/${TBB_ARCH}/vc14)

  # Copy TBB libraries to deploy and build folder. This is to support windows
  # QA. TBB is used by task_executor.
  set(TBB_BINARIES_BUILD_DIR
      ${CMAKE_CURRENT_BINARY_DIR}/lib/${OUTPUT_ARCH_SUFF})
  add_custom_target(
    tbb ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${TBB_LIBRARY_DIR}
            ${TBB_BINARIES_BUILD_DIR}
    COMMENT "Copying tbb libraries from ${TBB_LIBRARY_DIR} to ${TBB_BINARIES_BUILD_DIR}")

  install_to(${TBB_LIBRARY_DIR}/ COMPONENT ocl-task_executor DESTINATION
             ${CMAKE_INSTALL_PREFIX}/lib/${OUTPUT_ARCH_SUFF})
else(WIN32)
  set(TBB_LIB_DIR ${TBB_ROOT_DIR}/lib/${TBB_ARCH}/gcc4.8)
  set(TBB_LIBRARY_DIR ${TBB_LIB_DIR})
endif(WIN32)

if((NOT TBB_LIB_DIR) OR (NOT EXISTS ${TBB_LIB_DIR}))
  message(FATAL_ERROR "TBB library path is not found: ${TBB_LIB_DIR}")
endif()

set(TBB_INC_DIR ${TBB_ROOT_DIR}/include)

if((NOT TBB_INC_DIR) OR (NOT EXISTS ${TBB_INC_DIR}))
  message(FATAL_ERROR "TBB include path is not found: ${TBB_INC_DIR}")
endif()

set(TBB_INCLUDE_DIRS ${TBB_INC_DIR} utils/task_executor/enhancements)

# paths
add_llvm_external_project(opencl)
set(CL_API_HEADERS ${OpenCL_INCLUDE_DIR} ${OCL_SOURCE_DIR}/framework/inc)
set(CL_LOADER ${OpenCL_LOADER_DIR})

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

# An umbrella target for all tests inside current folder.
if(OPENCL_BE_INCLUDE_TESTS OR OPENCL_RT_INCLUDE_TESTS)
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

if(INCLUDE_CMRT)
  add_subdirectory(common_runtime)
endif(INCLUDE_CMRT)

if(OPENCL_RT_INCLUDE_TESTS)
  add_subdirectory(tests)
endif(OPENCL_RT_INCLUDE_TESTS)

if(OPENCL_BE_INCLUDE_TESTS OR OPENCL_RT_INCLUDE_TESTS)
  umbrella_lit_testsuite_end(check-ocl-all)
  set_target_properties(check-ocl-all PROPERTIES FOLDER "Tests")
endif()

if(NOT OPENCL_INTREE_BUILD)
  set(INTEL_DEPLOY_COMPONENTS
      ocl-clang_compiler${BUILD_PLATFORM}
      ocl-OclCpuBackEnd${BUILD_PLATFORM}
      ocl-builtins
      ocl-svml
      OpenCL-ICD
      ocl-intelocl${BUILD_PLATFORM}
      ocl-config
      ocl-cpu_device${BUILD_PLATFORM}
      ocl-task_executor${BUILD_PLATFORM}
      ocl-common_clang
      ocl-builtins-shared
      ocl-cl_logger
      ocl-gdb-plugin)

  if(WIN32)
    set(INTEL_DEPLOY_COMPONENTS
        ${INTEL_DEPLOY_COMPONENTS}
        ocl-${DEBUGGING_DLL_NAME}${DEBUGGING_DLL_VERSION})
  endif(WIN32)

  # This command is used for creating a fake dependency in order to force
  # another command(s) to execute. Its output is never produced as a result thus
  # each other command that depends on it will always be executed. We use this
  # to force running deploy installation regardless of manifest files state.
  add_custom_command(OUTPUT __force_it COMMAND "${CMAKE_COMMAND}" -E echo)

  if(DEFINED INTEL_DEPLOY_COMPONENTS)
    set(manifest_list)

    foreach(comp ${INTEL_DEPLOY_COMPONENTS})
      message(STATUS "Adding component ${comp} to deploy")

      set(manifest ${CMAKE_CURRENT_BINARY_DIR}/install_manifest_${comp}.txt)
      add_custom_command(
        OUTPUT ${manifest}
        COMMAND "${CMAKE_COMMAND}" "-DCMAKE_INSTALL_COMPONENT=${comp}" -P
                "${CMAKE_BINARY_DIR}/cmake_install.cmake"
        DEPENDS __force_it
        COMMENT "Deploying component ${comp}"
        USES_TERMINAL)
      list(APPEND manifest_list ${manifest})
    endforeach(comp)

    add_custom_target(deploy DEPENDS ${manifest_list})
  else()
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
endif()
