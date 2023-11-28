# INTEL CONFIDENTIAL
#
# Copyright (C) 2022 Intel Corporation
#
# This software and the related documents are Intel copyrighted materials, and
# your use of them is governed by the express license under which they were
# provided to you ("License"). Unless the License provides otherwise, you may not
# use, modify, copy, publish, distribute, disclose or transmit this software or
# the related documents without Intel's prior written permission.
#
# This software and the related documents are provided as is, with no express
# or implied warranties, other than those that are expressly stated in the
# License.

cmake_policy(VERSION 3.20)

# Download a prebuilt compiler archive and store the path to the directory in
# out_variable. If the build is on compiler infrastructure, this will instead
# check the NFS archive mount for the location of the archive, and use xarch to
# download it to the NFS mount if necessary.
#
# Parameters:
# - out_variable: the name of the variable to save the file path to
# - deploy_project: the branch of the compiler to use (with a "deploy_" prefix).
# - variant: the variant of the compiler to use
# - stamp: the build time of the compiler to use (in YYYYDDMM_HHMMSS form)
# - type: the buildtype of the compiler to use (e.g., linux_qa_release)
function(xarch out_variable deploy_project variant stamp type)
  # Strip deploy_ from the project name
  string(REGEX REPLACE "^deploy_" "" project "${deploy_project}")

  set(ARTIFACTORY_URL "https://af01p-fm.devtools.intel.com/artifactory/compilers-archive-fm-local/")

  # Try to use the archive on NFS if possible.
  # TODO: Consider trying to infer this environment variable if not found?
  if(DEFINED ENV{ICS_ARCHIVE_ROOT})
    set(filename $ENV{ICS_ARCHIVE_ROOT}/${deploy_project}/${variant}/${stamp}/build/${type})
    message(STATUS "Checking for file ${filename}")
    # If it's not available, use xarch to save a copy to the NFS share.
    if(NOT EXISTS ${filename})
      message(STATUS "File not found, running xarch to retrieve it")
      execute_process(COMMAND "xarch get -p ${project} -variant ${variant} -type ${type} -stamp ${stamp} -nocs")
    endif()
  else()
    # Otherwise, pull directory from the artifactory server.
    message(STATUS "ICS_ARCHIVE_ROOT not found, will use artifactory for deps.")
    include(FetchContent)
    FetchContent_Declare(
      xarch_${out_variable}
      URL "${ARTIFACTORY_URL}/${project}/${variant}/${stamp}/${type}.tgz")
    FetchContent_MakeAvailable(xarch_${out_variable})
    string(TOLOWER "xarch_${out_variable}-src" xarch_srcdir)
    set(filename "${FETCHCONTENT_BASE_DIR}/${xarch_srcdir}")
  endif()
  set(${out_variable} "${filename}" CACHE PATH "")
endfunction()

# Parse use_archive.txt into ARCHPROJ and ARCHVER variables.
set(icsconfig_dir ${CMAKE_SOURCE_DIR}/../../icsconfig)
file(STRINGS ${icsconfig_dir}/use_archive.txt ARCHIVE_VARS)
if(NOT "${ARCHIVE_VARS}" MATCHES "^([A-Za-z0-9_]+)[ \t\r\n]+([A-Za-z0-9_]+)$")
  message(FATAL_ERROR "use_archive.txt has invalid text: ${ARCHIVE_VARS}")
endif()
set(ARCHPROJ ${CMAKE_MATCH_1})
set(ARCHVER ${CMAKE_MATCH_2})

# TODO: Parse use_components.txt as well. For now, we're relying on ics set
# context ws build to put the results of this file in the environment.

# Select projects in xmain that can be compiled. Ideally, these should be just a
# simple addition to LLVM_ENABLE_PROJECTS, but the logic is more complicated for
# a few projects.
set(XMAIN_PROJECTS
  clang-tidy
  compiler-rt
  cpul0
  esimdemu
  fortran
  libclc
  libcxx
  libdev
  opencl
  openmp
  sycl
  sycl-fusion
)

set(XMAIN_DEFAULT_PROJECTS fortran clang-tidy compiler-rt openmp sycl libclc)
set(XMAIN_ENABLE_PROJECTS "" CACHE STRING
  "Semicolon-separated list of xmain projects to build (${XMAIN_PROJECTS})")
set(XMAIN_DISABLE_PROJECTS "" CACHE STRING
  "Semicolon-separated list of xmain projects to not build (${XMAIN_PROJECTS})")

set(enabled_projects ${XMAIN_DEFAULT_PROJECTS} ${XMAIN_ENABLE_PROJECTS})
list(REMOVE_DUPLICATES enabled_projects)
foreach(project ${XMAIN_DISABLE_PROJECTS})
  if(NOT "${project}" IN_LIST XMAIN_PROJECTS)
    message(FATAL_ERROR "${project} isn't a known xmain project: ${XMAIN_PROJECTS}")
  endif()
  list(REMOVE_ITEM enabled_projects ${project})
endforeach()

foreach(project ${XMAIN_PROJECTS})
  if("${project}" IN_LIST enabled_projects)
    message(STATUS "${project} xmain project is enabled")
  else()
    message(STATUS "${project} xmain project is disabled")
  endif()
endforeach()

function(add_llvm_project project_name)
  list(APPEND LLVM_ENABLE_PROJECTS ${project_name})
  set(LLVM_ENABLE_PROJECTS ${LLVM_ENABLE_PROJECTS} CACHE STRING "" FORCE)
endfunction()

function(add_llvm_runtime runtime_name)
  list(APPEND LLVM_ENABLE_RUNTIMES ${runtime_name})
  set(LLVM_ENABLE_RUNTIMES ${LLVM_ENABLE_RUNTIMES} CACHE STRING "" FORCE)
endfunction()

# add_external_project(name [path])
# path is relative to the llvm/ directory. If not defined, it is asumed to be
# the same as its actual name
# TODO: move external path directories into the llvm/CMakeLists.txt instead.
function(add_external_project project_name)
  list(APPEND LLVM_EXTERNAL_PROJECTS ${project_name})
  set(LLVM_EXTERNAL_PROJECTS ${LLVM_EXTERNAL_PROJECTS} CACHE STRING "" FORCE)
  string(TOUPPER "${project_name}" upper_proj)
  string(REPLACE "-" "_" upper_proj ${upper_proj})
  if(ARGC GREATER 1)
    set(project_path ${ARGV1})
  else()
    set(project_path ${project_name})
  endif()
  cmake_path(ABSOLUTE_PATH project_path BASE_DIRECTORY "${CMAKE_SOURCE_DIR}/.."
             NORMALIZE)
  set(LLVM_EXTERNAL_${upper_proj}_SOURCE_DIR ${project_path} CACHE PATH "")
endfunction()

# Add mandatory projects...
add_llvm_project(clang)
add_llvm_project(lld)
add_external_project(llvm-spirv)
add_external_project(xdev "../xdev")

# ... and optional projects.
foreach(project ${enabled_projects})
  if(NOT "${project}" IN_LIST XMAIN_PROJECTS)
    message(FATAL_ERROR "${project} isn't a known xmain project: ${XMAIN_PROJECTS}")
  endif()

  if(project STREQUAL "clang-tidy")
    add_llvm_project(clang-tools-extra)
  elseif(project STREQUAL "compiler-rt")
    add_llvm_project(compiler-rt)
    set(COMPILER_RT_USE_LIBCXX OFF CACHE BOOL "")
  elseif(project STREQUAL "cpul0")
    add_external_project(llvm-zero-cpu-rt)
  elseif(project STREQUAL "esimdemu")
    set(SYCL_ENABLE_PLUGINS "opencl;level_zero;esimd_emulator" CACHE STRING "")
  elseif(project STREQUAL "fortran")
    set(BUILD_FORT ON CACHE BOOL "")
  elseif(project STREQUAL "libclc")
    add_llvm_project(libclc)
    set(LIBCLC_TARGETS_TO_BUILD
      "amdgcn--"
      "amdgcn--amdhsa"
      "nvptx64--"
      "nvptx64--nvidiacl"
      CACHE STRING "")
    set(LIBCLC_GENERATE_REMANGLED_VARIANTS ON CACHE BOOL "")
  elseif(project STREQUAL "libcxx")
    add_llvm_runtime(libcxx)
    add_llvm_runtime(libcxxabi)
  elseif(project STREQUAL "libdev")
    add_external_project(libdev "../libdev")
    set(IL0_COMPILER "$ENV{IL0_COMPILER}" CACHE PATH "")
  elseif(project STREQUAL "opencl")
    add_external_project(opencl)
    add_external_project(opencl-intel)
    # Extra options....
    set(OCL_RELEASE_BRANCH "$ENV{ICS_WSPROJECT}" CACHE STRING "")
    set(OCL_REVISION "$ENV{ICS_WSVERSION}" CACHE STRING "")
    set(OPENCL_INTREE_BUILD ON CACHE BOOL "")
    set(INCLUDE_MKL OFF CACHE BOOL "")
    set(CMAKE_SKIP_RPATH OFF CACHE BOOL "")
    set(ENABLE_SDE OFF CACHE BOOL "")
    if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
      set(USE_VALGRIND ON CACHE BOOL "")
      set(BACKEND_BUILD_VERIFICATION_LIB ON CACHE BOOL "")
    else()
      set(USE_VALGRIND OFF CACHE BOOL "")
      set(BACKEND_BUILD_VERIFICATION_LIB OFF CACHE BOOL "")
      set(LLVM_REVISION_BE OCL_LLVM_INTEL_FPGA_Q3_1 CACHE STRING "")
      set(LLVM_REVISION_FE OCL_LLVM_INTEL_FPGA_Q3_1 CACHE STRING "")
    endif()
  elseif(project STREQUAL "openmp")
    add_llvm_project(openmp)
    if(DEFINED ENV{LIBOMPTARGET_DEP_MPSS_SDK})
      set(LIBOMPTARGET_DEP_MPSS_SDK "$ENV{LIBOMPTARGET_DEP_MPSS_SDK}" CACHE STRING "")
    endif()
  elseif(project STREQUAL "sycl")
    add_llvm_project(aocl-ioc64)
    add_llvm_project(libdevice)
    add_llvm_project(opencl)
    add_external_project(aocl-ioc64)
    add_external_project(opencl)
    add_external_project(libdevice)
    add_external_project(sycl)
    add_external_project(xpti)
    add_external_project(xptifw)
    set(SYCL_ENABLE_XPTI_TRACING ON CACHE BOOL "")
    # TODO: Pull this information from use_components.txt directly instead of
    # the ics build environment.
    if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
      set(L0LOADER $ENV{L0LOADERROOT}/$ENV{L0LOADERVER})
      set(L0_INCLUDE_DIR ${L0LOADER}/include CACHE PATH "")
      set(L0_LIBRARY ${L0LOADER}/lib/ze_loader.lib CACHE PATH "")
    else()
      set(L0GPU $ENV{L0GPUROOT}/$ENV{L0GPUVER})
      set(L0_INCLUDE_DIR ${L0GPU}/include CACHE PATH "")
      set(L0_LIBRARY ${L0GPU}/libze_loader.so CACHE PATH "")
    endif()
    set(SYCL_ENABLE_KERNEL_FUSION OFF CACHE BOOL "")
  elseif(project STREQUAL "sycl-fusion")
    add_external_project(sycl-fusion)
    set(SYCL_ENABLE_KERNEL_FUSION ON CACHE BOOL "" FORCE)
  endif()
endforeach()

# xdev options.
# TODO: remove these options.
set(HOST_ARCH EFI2 CACHE STRING "")
set(TARGET_ARCH EFI2 CACHE STRING "")
set(USER $ENV{ICS_USER} CACHE STRING "")
set(SYSTEM $ENV{ICS_NODE} CACHE STRING "")
set(DIR $ENV{ICS_WSDIR} CACHE FILEPATH "")
set(LRB_SUPPORT "DISABLED" CACHE BOOL "")
set(BUILD_SV OFF CACHE BOOL "")

# Other LLVM options.
set(LLVM_TARGETS_TO_BUILD "X86;NVPTX;AMDGPU" CACHE STRING "")
set(LLVM_ENABLE_LIBPFM OFF CACHE BOOL "")
set(INTEL_CMAKE_DEPLOY ON CACHE BOOL "")
set(UWD_BUILD ON CACHE BOOL "")

# onnx install location
# TODO: Pull this information from use_components.txt directly instead of the
# ics build environment.
set(onnxruntime_INSTALL_PREFIX $ENV{ONNXRTROOT}/$ENV{ONNXRTVER} CACHE STRING "")
set(ONNX_MODEL_PATH $ENV{ONNXMDLROOT}/$ENV{ONNXMDLVER} CACHE STRING "")

# Grab the prebuilt-components archive.
set(IL0_PREBUILT_COMPONENTS_STRUCTURE "install" CACHE STRING "")

function(add_cflags flags)
  if(DEFINED CMAKE_C_FLAGS OR DEFINED CMAKE_CXX_FLAGS)
    set(CMAKE_C_FLAGS "${flags} ${CMAKE_C_FLAGS}" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS "${flags} ${CMAKE_CXX_FLAGS}" CACHE STRING "" FORCE)
  else()
    set(CMAKE_C_FLAGS "${flags}" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS "${flags}" CACHE STRING "" FORCE)
  endif()
endfunction()

# Include OS-specific options.
include(${CMAKE_CURRENT_LIST_DIR}/${CMAKE_HOST_SYSTEM_NAME}.cmake)

# If we're compiling OpenCL, copy the C compiler as the ASM compiler as well.
# TODO: Remove the need for this step.
if(opencl IN_LIST enabled_projects)
  set(OPENCL_ASM_COMPILER "${CMAKE_C_COMPILER}" CACHE PATH "")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "")
