# INTEL_CUSTOMIZATION
#
# INTEL CONFIDENTIAL
#
# Modifications, Copyright (C) 2021-2022 Intel Corporation
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
#
# end INTEL_CUSTOMIZATION
# See docs/CMake.html for instructions about how to build LLVM with CMake.

cmake_minimum_required(VERSION 3.13.4)

# CMP0114: ExternalProject step targets fully adopt their steps.
# New in CMake 3.19: https://cmake.org/cmake/help/latest/policy/CMP0114.html
if(POLICY CMP0114)
  cmake_policy(SET CMP0114 OLD)
endif()
# CMP0116: Ninja generators transform `DEPFILE`s from `add_custom_command()`
# New in CMake 3.20. https://cmake.org/cmake/help/latest/policy/CMP0116.html
if(POLICY CMP0116)
  cmake_policy(SET CMP0116 OLD)
endif()

set(CMAKE_BUILD_WITH_INSTALL_NAME_DIR ON)

if(NOT DEFINED LLVM_VERSION_MAJOR)
  set(LLVM_VERSION_MAJOR 16)
endif()
if(NOT DEFINED LLVM_VERSION_MINOR)
  set(LLVM_VERSION_MINOR 0)
endif()
if(NOT DEFINED LLVM_VERSION_PATCH)
  set(LLVM_VERSION_PATCH 0)
endif()
if(NOT DEFINED LLVM_VERSION_SUFFIX)
  set(LLVM_VERSION_SUFFIX git)
endif()

if (NOT PACKAGE_VERSION)
  set(PACKAGE_VERSION
    "${LLVM_VERSION_MAJOR}.${LLVM_VERSION_MINOR}.${LLVM_VERSION_PATCH}${LLVM_VERSION_SUFFIX}")
endif()

if(NOT DEFINED LLVM_SHLIB_SYMBOL_VERSION)
  # "Symbol version prefix for libLLVM.so"
  set(LLVM_SHLIB_SYMBOL_VERSION "LLVM_${LLVM_VERSION_MAJOR}")
endif()

set(INTEL_CUSTOMIZATION ON)

if ((CMAKE_GENERATOR MATCHES "Visual Studio") AND (MSVC_TOOLSET_VERSION LESS 142) AND (CMAKE_GENERATOR_TOOLSET STREQUAL ""))
  message(WARNING "Visual Studio generators use the x86 host compiler by "
                  "default, even for 64-bit targets. This can result in linker "
                  "instability and out of memory errors. To use the 64-bit "
                  "host compiler, pass -Thost=x64 on the CMake command line.")
endif()

if (CMAKE_GENERATOR STREQUAL "Xcode" AND NOT CMAKE_OSX_ARCHITECTURES)
  # Some CMake features like object libraries get confused if you don't
  # explicitly specify an architecture setting with the Xcode generator.
  set(CMAKE_OSX_ARCHITECTURES "x86_64")
endif()

project(LLVM
  VERSION ${LLVM_VERSION_MAJOR}.${LLVM_VERSION_MINOR}.${LLVM_VERSION_PATCH}
  LANGUAGES C CXX ASM)

if (NOT DEFINED CMAKE_INSTALL_LIBDIR AND DEFINED LLVM_LIBDIR_SUFFIX)
  # Must go before `include(GNUInstallDirs)`.
  set(CMAKE_INSTALL_LIBDIR "lib${LLVM_LIBDIR_SUFFIX}")
endif()

# Must go after `DEFINED LLVM_LIBDIR_SUFFIX` check.
set(LLVM_LIBDIR_SUFFIX "" CACHE STRING "Define suffix of library directory name (32/64)" )

if (INTEL_CUSTOMIZATION)
  set(DPCPP_VERSION_QUALITY " ")
  set(DPCPP_VERSION_MAJOR "2023")
  set(DPCPP_VERSION_MINOR "0")
  set(DPCPP_VERSION_PATCH "0")
  set(VERSION "${DPCPP_VERSION_MAJOR}.${DPCPP_VERSION_MINOR}.${DPCPP_VERSION_PATCH}")
  set(DPCPP_PRODUCT_VERSION ${VERSION})
  set(FULL_VERSION ${DPCPP_PRODUCT_VERSION})
  if(NOT DPCPP_VERSION_QUALITY STREQUAL " ")
     set(FULL_VERSION "${FULL_VERSION}-${DPCPP_VERSION_QUALITY}")
  endif()
  set(DPCPP_PRODUCT_FULL_VERSION ${FULL_VERSION})
  set(DPCPP_PRODUCT_NAME "Intel(R) oneAPI DPC++/C++ Compiler")
  # The bitcode producer string must be encoded using only alphanumeric
  # characters plus '.' and '_'
  set(DPCPP_BITCODE_PRODUCER_STR "Intel.oneAPI.DPCPP.Compiler_${DPCPP_PRODUCT_VERSION}")
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/llvm/Config/dpcpp.version.info.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/llvm/Config/dpcpp.version.info.h
    )
endif (INTEL_CUSTOMIZATION)

# Must go after project(..)
include(GNUInstallDirs)

# This C++ standard is required to build LLVM.
set(LLVM_REQUIRED_CXX_STANDARD 17)

# If we find that the cache contains CMAKE_CXX_STANDARD it means that it's a old CMakeCache.txt
# and we can just inform the user and then reset it.
if($CACHE{CMAKE_CXX_STANDARD} AND $CACHE{CMAKE_CXX_STANDARD} LESS ${LLVM_REQUIRED_CXX_STANDARD})
  message(WARNING "Resetting cache value for CMAKE_CXX_STANDARD to ${LLVM_REQUIRED_CXX_STANDARD}")
  unset(CMAKE_CXX_STANDARD CACHE)
endif()

# if CMAKE_CXX_STANDARD is still set after the cache unset above it means that the user requested it
# and we allow it to be set to something newer than the required standard but otherwise we fail.
if(DEFINED CMAKE_CXX_STANDARD AND CMAKE_CXX_STANDARD LESS ${LLVM_REQUIRED_CXX_STANDARD})
  message(FATAL_ERROR "Requested CMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD} which is less than the required ${LLVM_REQUIRED_CXX_STANDARD}.")
endif()

set(CMAKE_CXX_STANDARD ${LLVM_REQUIRED_CXX_STANDARD} CACHE STRING "C++ standard to conform to")
set(CMAKE_CXX_STANDARD_REQUIRED YES)

if (CYGWIN)
  # Cygwin is a bit stricter and lack things like 'strdup', 'stricmp', etc in
  # c++xx mode.
  set(CMAKE_CXX_EXTENSIONS YES)
else()
  set(CMAKE_CXX_EXTENSIONS NO)
endif()

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(FATAL_ERROR "
No build type selected. You need to pass -DCMAKE_BUILD_TYPE=<type> in order to configure LLVM.
Available options are:
  * -DCMAKE_BUILD_TYPE=Release - For an optimized build with no assertions or debug info.
  * -DCMAKE_BUILD_TYPE=Debug - For an unoptimized build with assertions and debug info.
  * -DCMAKE_BUILD_TYPE=RelWithDebInfo - For an optimized build with no assertions but with debug info.
  * -DCMAKE_BUILD_TYPE=MinSizeRel - For a build optimized for size instead of speed.
Learn more about these options in our documentation at https://llvm.org/docs/CMake.html#cmake-build-type
")
endif()

# Side-by-side subprojects layout: automatically set the
# LLVM_EXTERNAL_${project}_SOURCE_DIR using LLVM_ALL_PROJECTS
# This allows an easy way of setting up a build directory for llvm and another
# one for llvm+clang+... using the same sources.
set(LLVM_ALL_PROJECTS "bolt;clang;clang-tools-extra;compiler-rt;cross-project-tests;libc;libclc;libcxx;libcxxabi;libunwind;lld;lldb;mlir;openmp;polly;pstl")
set(LLVM_ALL_PROJECTS "${LLVM_ALL_PROJECTS};${LLVM_EXTERNAL_PROJECTS}")
# The flang project is not yet part of "all" projects (see C++ requirements)
set(LLVM_EXTRA_PROJECTS "flang")
# List of all known projects in the mono repo
set(LLVM_KNOWN_PROJECTS "${LLVM_ALL_PROJECTS};${LLVM_EXTRA_PROJECTS}")
if (INTEL_CUSTOMIZATION)
  # List of Intel specific projects
  set(LLVM_INTEL_EXTRA_PROJECTS "opencl;aocl-ioc64;libdevice")
  # List of all known projects in the mono repo
  set(LLVM_KNOWN_PROJECTS "${LLVM_ALL_PROJECTS};${LLVM_EXTRA_PROJECTS};${LLVM_INTEL_EXTRA_PROJECTS}")
endif (INTEL_CUSTOMIZATION)
set(LLVM_ENABLE_PROJECTS "" CACHE STRING
    "Semicolon-separated list of projects to build (${LLVM_KNOWN_PROJECTS}), or \"all\".")
# Make sure expansion happens first to not handle "all" in rest of the checks.
if( LLVM_ENABLE_PROJECTS STREQUAL "all" )
  set( LLVM_ENABLE_PROJECTS ${LLVM_ALL_PROJECTS})
endif()
foreach(proj ${LLVM_ENABLE_PROJECTS})
  if (NOT proj STREQUAL "llvm" AND NOT "${proj}" IN_LIST LLVM_KNOWN_PROJECTS)
     MESSAGE(FATAL_ERROR "${proj} isn't a known project: ${LLVM_KNOWN_PROJECTS}")
  endif()
endforeach()
foreach(proj "libcxx" "libcxxabi" "libunwind")
  if (${proj} IN_LIST LLVM_ENABLE_PROJECTS)
    message(WARNING "Using LLVM_ENABLE_PROJECTS=${proj} is deprecated now, please use -DLLVM_ENABLE_RUNTIMES=${proj} or "
                    "see the instructions at https://libcxx.llvm.org/BuildingLibcxx.html for building the runtimes.")
  endif()
endforeach()

if ("flang" IN_LIST LLVM_ENABLE_PROJECTS)
  if (NOT "mlir" IN_LIST LLVM_ENABLE_PROJECTS)
    message(STATUS "Enabling MLIR as a dependency to flang")
    list(APPEND LLVM_ENABLE_PROJECTS "mlir")
  endif()

  if (NOT "clang" IN_LIST LLVM_ENABLE_PROJECTS)
    message(FATAL_ERROR "Clang is not enabled, but is required for the Flang driver")
  endif()
endif()

# LLVM_ENABLE_PROJECTS_USED is `ON` if the user has ever used the
# `LLVM_ENABLE_PROJECTS` CMake cache variable.  This exists for
# several reasons:
#
# * As an indicator that the `LLVM_ENABLE_PROJECTS` list is now the single
# source of truth for which projects to build. This means we will ignore user
# supplied `LLVM_TOOL_<project>_BUILD` CMake cache variables and overwrite
# them.
#
# * The case where the user previously had `LLVM_ENABLE_PROJECTS` set to a
# non-empty list but now the user wishes to disable building all other projects
# by setting `LLVM_ENABLE_PROJECTS` to an empty string. In that case we still
# need to set the `LLVM_TOOL_${upper_proj}_BUILD` variables so that we disable
# building all the projects that were previously enabled.
set(LLVM_ENABLE_PROJECTS_USED OFF CACHE BOOL "")
mark_as_advanced(LLVM_ENABLE_PROJECTS_USED)

if (LLVM_ENABLE_PROJECTS_USED OR NOT LLVM_ENABLE_PROJECTS STREQUAL "")
  set(LLVM_ENABLE_PROJECTS_USED ON CACHE BOOL "" FORCE)
  foreach(proj ${LLVM_KNOWN_PROJECTS} ${LLVM_EXTERNAL_PROJECTS})
    string(TOUPPER "${proj}" upper_proj)
    string(REGEX REPLACE "-" "_" upper_proj ${upper_proj})
    if ("${proj}" IN_LIST LLVM_ENABLE_PROJECTS)
      message(STATUS "${proj} project is enabled")
      set(SHOULD_ENABLE_PROJECT TRUE)
      set(PROJ_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../${proj}")
      if(NOT EXISTS "${PROJ_DIR}" OR NOT IS_DIRECTORY "${PROJ_DIR}")
        message(FATAL_ERROR "LLVM_ENABLE_PROJECTS requests ${proj} but directory not found: ${PROJ_DIR}")
      endif()
      if( LLVM_EXTERNAL_${upper_proj}_SOURCE_DIR STREQUAL "" )
        set(LLVM_EXTERNAL_${upper_proj}_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../${proj}" CACHE PATH "" FORCE)
      else()
        set(LLVM_EXTERNAL_${upper_proj}_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../${proj}" CACHE PATH "")
      endif()
    elseif ("${proj}" IN_LIST LLVM_EXTERNAL_PROJECTS)
      message(STATUS "${proj} project is enabled")
      set(SHOULD_ENABLE_PROJECT TRUE)
    else()
      message(STATUS "${proj} project is disabled")
      set(SHOULD_ENABLE_PROJECT FALSE)
    endif()
    # Force `LLVM_TOOL_${upper_proj}_BUILD` variables to have values that
    # corresponds with `LLVM_ENABLE_PROJECTS`. This prevents the user setting
    # `LLVM_TOOL_${upper_proj}_BUILD` variables externally. At some point
    # we should deprecate allowing users to set these variables by turning them
    # into normal CMake variables rather than cache variables.
    set(LLVM_TOOL_${upper_proj}_BUILD
      ${SHOULD_ENABLE_PROJECT}
      CACHE
      BOOL "Whether to build ${upper_proj} as part of LLVM" FORCE
    )
  endforeach()
endif()
unset(SHOULD_ENABLE_PROJECT)

# Build llvm with ccache if the package is present
set(LLVM_CCACHE_BUILD OFF CACHE BOOL "Set to ON for a ccache enabled build")
if(LLVM_CCACHE_BUILD)
  find_program(CCACHE_PROGRAM ccache)
  if(CCACHE_PROGRAM)
    set(LLVM_CCACHE_MAXSIZE "" CACHE STRING "Size of ccache")
    set(LLVM_CCACHE_DIR "" CACHE STRING "Directory to keep ccached data")
    set(LLVM_CCACHE_PARAMS "CCACHE_CPP2=yes CCACHE_HASHDIR=yes"
        CACHE STRING "Parameters to pass through to ccache")

    if(NOT CMAKE_SYSTEM_NAME MATCHES "Windows")
      set(CCACHE_PROGRAM "${LLVM_CCACHE_PARAMS} ${CCACHE_PROGRAM}")
      if (LLVM_CCACHE_MAXSIZE)
        set(CCACHE_PROGRAM "CCACHE_MAXSIZE=${LLVM_CCACHE_MAXSIZE} ${CCACHE_PROGRAM}")
      endif()
      if (LLVM_CCACHE_DIR)
        set(CCACHE_PROGRAM "CCACHE_DIR=${LLVM_CCACHE_DIR} ${CCACHE_PROGRAM}")
      endif()
      set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${CCACHE_PROGRAM})
    else()
      if(LLVM_CCACHE_MAXSIZE OR LLVM_CCACHE_DIR OR
         NOT LLVM_CCACHE_PARAMS MATCHES "CCACHE_CPP2=yes CCACHE_HASHDIR=yes")
        message(FATAL_ERROR "Ccache configuration through CMake is not supported on Windows. Please use environment variables.")
      endif()
      # RULE_LAUNCH_COMPILE should work with Ninja but currently has issues
      # with cmd.exe and some MSVC tools other than cl.exe
      set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
      set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
    endif()
  else()
    message(FATAL_ERROR "Unable to find the program ccache. Set LLVM_CCACHE_BUILD to OFF")
  endif()
endif()

set(LLVM_EXTERNAL_PROJECT_BUILD_TOOL_ARGS "" CACHE STRING
  "Optional arguments for the native tool used in CMake --build invocations for external projects.")
mark_as_advanced(LLVM_EXTERNAL_PROJECT_BUILD_TOOL_ARGS)

option(LLVM_DEPENDENCY_DEBUGGING "Dependency debugging mode to verify correctly expressed library dependencies (Darwin only)" OFF)

# Some features of the LLVM build may be disallowed when dependency debugging is
# enabled. In particular you cannot use ccache because we want to force compile
# operations to always happen.
if(LLVM_DEPENDENCY_DEBUGGING)
  if(NOT CMAKE_HOST_APPLE)
    message(FATAL_ERROR "Dependency debugging is only currently supported on Darwin hosts.")
  endif()
  if(LLVM_CCACHE_BUILD)
    message(FATAL_ERROR "Cannot enable dependency debugging while using ccache.")
  endif()
endif()

option(LLVM_ENABLE_DAGISEL_COV "Debug: Prints tablegen patterns that were used for selecting" OFF)
option(LLVM_ENABLE_GISEL_COV "Enable collection of GlobalISel rule coverage" OFF)
if(LLVM_ENABLE_GISEL_COV)
  set(LLVM_GISEL_COV_PREFIX "${CMAKE_BINARY_DIR}/gisel-coverage-" CACHE STRING "Provide a filename prefix to collect the GlobalISel rule coverage")
endif()

set(LLVM_COMMON_CMAKE_UTILS ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)

# Add path for custom modules
list(INSERT CMAKE_MODULE_PATH 0
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
  "${LLVM_COMMON_CMAKE_UTILS}/Modules"
  )

# Generate a CompilationDatabase (compile_commands.json file) for our build,
# for use by clang_complete, YouCompleteMe, etc.
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

option(LLVM_INSTALL_BINUTILS_SYMLINKS
  "Install symlinks from the binutils tool names to the corresponding LLVM tools." OFF)

option(LLVM_INSTALL_CCTOOLS_SYMLINKS
  "Install symlinks from the cctools tool names to the corresponding LLVM tools." OFF)

option(LLVM_INSTALL_UTILS "Include utility binaries in the 'install' target." OFF)

option(LLVM_INSTALL_TOOLCHAIN_ONLY "Only include toolchain files in the 'install' target." OFF)

# Unfortunatly Clang is too eager to search directories for module maps, which can cause the
# installed version of the maps to be found when building LLVM from source. Therefore we turn off
# the installation by default. See llvm.org/PR31905.
option(LLVM_INSTALL_MODULEMAPS "Install the modulemap files in the 'install' target." OFF)

option(LLVM_USE_FOLDERS "Enable solution folders in Visual Studio. Disable for Express versions." ON)
if ( LLVM_USE_FOLDERS )
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

include(VersionFromVCS)

option(LLVM_APPEND_VC_REV
  "Embed the version control system revision in LLVM" ON)

option(LLVM_TOOL_LLVM_DRIVER_BUILD "Enables building the llvm multicall tool" OFF)

set(PACKAGE_NAME LLVM)
set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set(PACKAGE_BUGREPORT "https://github.com/llvm/llvm-project/issues/")

if (INTEL_CUSTOMIZATION)
  set(PACKAGE_BUGREPORT "https://software.intel.com/en-us/support/priority-support")
endif (INTEL_CUSTOMIZATION)

set(BUG_REPORT_URL "${PACKAGE_BUGREPORT}" CACHE STRING
  "Default URL where bug reports are to be submitted.")

# Configure CPack.
set(CPACK_PACKAGE_INSTALL_DIRECTORY "LLVM")
set(CPACK_PACKAGE_VENDOR "LLVM")
set(CPACK_PACKAGE_VERSION_MAJOR ${LLVM_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${LLVM_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${LLVM_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION ${PACKAGE_VERSION})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.TXT")
set(CPACK_NSIS_COMPRESSOR "/SOLID lzma \r\n SetCompressorDictSize 32")
if(WIN32 AND NOT UNIX)
  set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "LLVM")
  set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}\\\\cmake\\\\nsis_logo.bmp")
  set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}\\\\cmake\\\\nsis_icon.ico")
  set(CPACK_NSIS_MUI_UNIICON "${CMAKE_CURRENT_SOURCE_DIR}\\\\cmake\\\\nsis_icon.ico")
  set(CPACK_NSIS_MODIFY_PATH "ON")
  set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL "ON")
  if( CMAKE_CL_64 )
    set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
  endif()
endif()
include(CPack)

# Sanity check our source directory to make sure that we are not trying to
# generate an in-source build (unless on MSVC_IDE, where it is ok), and to make
# sure that we don't have any stray generated files lying around in the tree
# (which would end up getting picked up by header search, instead of the correct
# versions).
if( CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR AND NOT MSVC_IDE )
  message(FATAL_ERROR "In-source builds are not allowed.
Please create a directory and run cmake from there, passing the path
to this source directory as the last argument.
This process created the file `CMakeCache.txt' and the directory `CMakeFiles'.
Please delete them.")
endif()

string(TOUPPER "${CMAKE_BUILD_TYPE}" uppercase_CMAKE_BUILD_TYPE)

if (CMAKE_BUILD_TYPE AND
    NOT uppercase_CMAKE_BUILD_TYPE MATCHES "^(DEBUG|RELEASE|RELWITHDEBINFO|MINSIZEREL)$")
  message(FATAL_ERROR "Invalid value for CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
endif()

# LLVM_INSTALL_PACKAGE_DIR needs to be declared prior to adding the tools
# subdirectory in order to have the value available for llvm-config.
include(GNUInstallPackageDir)
set(LLVM_INSTALL_PACKAGE_DIR "${CMAKE_INSTALL_PACKAGEDIR}/llvm" CACHE STRING
  "Path for CMake subdirectory for LLVM (defaults to '${CMAKE_INSTALL_PACKAGEDIR}/llvm')")

# INTEL_CUSTOMIZATION

# This is a transitional option which enables the new unified 2023 OneAPI
# directory layout as per PREQS-1605. Currently the ICS "xmain" project builds
# with this disabled, and "xmain-2023" builds with it enabled.
set(INTEL_DEPLOY_UNIFIED_LAYOUT OFF CACHE BOOL "Build and deploy for a unified OneAPI directory layout")

if(INTEL_DEPLOY_UNIFIED_LAYOUT)
  add_compile_definitions(INTEL_DEPLOY_UNIFIED_LAYOUT)
  set(INTEL_DEPLOY_PKGCOMP_NAME "compiler" CACHE STRING "OneAPI deployment package component name")
  set(LLVM_TOOLS_INSTALL_DIR
    "${CMAKE_INSTALL_BINDIR}/${INTEL_DEPLOY_PKGCOMP_NAME}"
    CACHE STRING "Path for binary subdirectory")
else()
  set(LLVM_TOOLS_INSTALL_DIR "bin-llvm" CACHE STRING "Path for binary subdirectory (defaults to 'bin-llvm')")
endif()

# If configured to install a separate tools directory (as is the default)
# install a README explaining its purpose.
if(NOT LLVM_TOOLS_INSTALL_DIR STREQUAL "bin")
  install(FILES "intel-bin-llvm-README.txt"
    DESTINATION ${LLVM_TOOLS_INSTALL_DIR}
    COMPONENT bin-llvm-readme
    RENAME "README.txt"
    )
endif()
# end INTEL_CUSTOMIZATION

mark_as_advanced(LLVM_TOOLS_INSTALL_DIR)

set(LLVM_UTILS_INSTALL_DIR "${LLVM_TOOLS_INSTALL_DIR}" CACHE STRING
    "Path to install LLVM utilities (enabled by LLVM_INSTALL_UTILS=ON) (defaults to LLVM_TOOLS_INSTALL_DIR)")
mark_as_advanced(LLVM_UTILS_INSTALL_DIR)

set(LLVM_EXAMPLES_INSTALL_DIR "examples" CACHE STRING
    "Path for examples subdirectory (enabled by LLVM_BUILD_EXAMPLES=ON) (defaults to 'examples')")
mark_as_advanced(LLVM_EXAMPLES_INSTALL_DIR)

# They are used as destination of target generators.
set(LLVM_RUNTIME_OUTPUT_INTDIR ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/bin)
set(LLVM_LIBRARY_OUTPUT_INTDIR ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/lib${LLVM_LIBDIR_SUFFIX})
if(WIN32 OR CYGWIN)
  # DLL platform -- put DLLs into bin.
  set(LLVM_SHLIB_OUTPUT_INTDIR ${LLVM_RUNTIME_OUTPUT_INTDIR})
else()
  set(LLVM_SHLIB_OUTPUT_INTDIR ${LLVM_LIBRARY_OUTPUT_INTDIR})
endif()

# Each of them corresponds to llvm-config's.
set(LLVM_TOOLS_BINARY_DIR ${LLVM_RUNTIME_OUTPUT_INTDIR}) # --bindir
set(LLVM_LIBRARY_DIR      ${LLVM_LIBRARY_OUTPUT_INTDIR}) # --libdir
set(LLVM_MAIN_SRC_DIR     ${CMAKE_CURRENT_SOURCE_DIR}  ) # --src-root
set(LLVM_MAIN_INCLUDE_DIR ${LLVM_MAIN_SRC_DIR}/include ) # --includedir
set(LLVM_BINARY_DIR       ${CMAKE_CURRENT_BINARY_DIR}  ) # --prefix

set(LLVM_THIRD_PARTY_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/../third-party)

# Note: LLVM_CMAKE_DIR does not include generated files
set(LLVM_CMAKE_DIR ${LLVM_MAIN_SRC_DIR}/cmake/modules)
set(LLVM_EXAMPLES_BINARY_DIR ${LLVM_BINARY_DIR}/examples)
set(LLVM_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/include)

# List of all targets to be built by default:
set(LLVM_ALL_TARGETS
  AArch64
  AMDGPU
  ARM
  AVR
  BPF
# INTEL_CUSTOMIZATION
# INTEL_FEATURE_CSA
  CSA
# end INTEL_FEATURE_CSA
# end INTEL_CUSTOMIZATION
  Hexagon
  Lanai
  Mips
  MSP430
  NVPTX
  PowerPC
  RISCV
  Sparc
  SystemZ
  VE
  WebAssembly
  X86
  XCore
  )

# List of targets with JIT support:
set(LLVM_TARGETS_WITH_JIT X86 PowerPC AArch64 ARM Mips SystemZ)

set(LLVM_TARGETS_TO_BUILD "all"
    CACHE STRING "Semicolon-separated list of targets to build, or \"all\".")

set(LLVM_EXPERIMENTAL_TARGETS_TO_BUILD ""
    CACHE STRING "Semicolon-separated list of experimental targets to build.")

option(BUILD_SHARED_LIBS
  "Build all libraries as shared libraries instead of static" OFF)

option(LLVM_ENABLE_BACKTRACES "Enable embedding backtraces on crash." ON)
if(LLVM_ENABLE_BACKTRACES)
  set(ENABLE_BACKTRACES 1)
endif()

option(LLVM_ENABLE_UNWIND_TABLES "Emit unwind tables for the libraries" ON)

option(LLVM_ENABLE_CRASH_OVERRIDES "Enable crash overrides." ON)
if(LLVM_ENABLE_CRASH_OVERRIDES)
  set(ENABLE_CRASH_OVERRIDES 1)
endif()

option(LLVM_ENABLE_CRASH_DUMPS "Turn on memory dumps on crashes. Currently only implemented on Windows." OFF)

set(WINDOWS_PREFER_FORWARD_SLASH_DEFAULT OFF)
if (MINGW)
  # Cygwin doesn't identify itself as Windows, and thus gets path::Style::posix
  # as native path style, regardless of what this is set to.
  set(WINDOWS_PREFER_FORWARD_SLASH_DEFAULT ON)
endif()
option(LLVM_WINDOWS_PREFER_FORWARD_SLASH "Prefer path names with forward slashes on Windows." ${WINDOWS_PREFER_FORWARD_SLASH_DEFAULT})

option(LLVM_ENABLE_FFI "Use libffi to call external functions from the interpreter" OFF)
set(FFI_LIBRARY_DIR "" CACHE PATH "Additional directory, where CMake should search for libffi.so")
set(FFI_INCLUDE_DIR "" CACHE PATH "Additional directory, where CMake should search for ffi.h or ffi/ffi.h")

set(LLVM_TARGET_ARCH "host"
  CACHE STRING "Set target to use for LLVM JIT or use \"host\" for automatic detection.")

option(LLVM_ENABLE_TERMINFO "Use terminfo database if available." ON)

set(LLVM_ENABLE_LIBXML2 "ON" CACHE STRING "Use libxml2 if available. Can be ON, OFF, or FORCE_ON")

option(LLVM_ENABLE_LIBEDIT "Use libedit if available." ON)

option(LLVM_ENABLE_LIBPFM "Use libpfm for performance counters if available." ON)

# On z/OS, threads cannot be used because TLS is not supported.
if (CMAKE_SYSTEM_NAME MATCHES "OS390")
  option(LLVM_ENABLE_THREADS "Use threads if available." OFF)
else()
  option(LLVM_ENABLE_THREADS "Use threads if available." ON)
endif()

set(LLVM_ENABLE_ZLIB "ON" CACHE STRING "Use zlib for compression/decompression if available. Can be ON, OFF, or FORCE_ON")

set(LLVM_ENABLE_ZSTD "ON" CACHE STRING "Use zstd for compression/decompression if available. Can be ON, OFF, or FORCE_ON")

set(LLVM_ENABLE_CURL "OFF" CACHE STRING "Use libcurl for the HTTP client if available. Can be ON, OFF, or FORCE_ON")

set(LLVM_ENABLE_HTTPLIB "OFF" CACHE STRING "Use cpp-httplib HTTP server library if available. Can be ON, OFF, or FORCE_ON")

set(LLVM_Z3_INSTALL_DIR "" CACHE STRING "Install directory of the Z3 solver.")

option(LLVM_ENABLE_Z3_SOLVER
  "Enable Support for the Z3 constraint solver in LLVM."
  ${LLVM_ENABLE_Z3_SOLVER_DEFAULT}
)

if (LLVM_ENABLE_Z3_SOLVER)
  find_package(Z3 4.7.1)

  if (LLVM_Z3_INSTALL_DIR)
    if (NOT Z3_FOUND)
      message(FATAL_ERROR "Z3 >= 4.7.1 has not been found in LLVM_Z3_INSTALL_DIR: ${LLVM_Z3_INSTALL_DIR}.")
    endif()
  endif()

  if (NOT Z3_FOUND)
    message(FATAL_ERROR "LLVM_ENABLE_Z3_SOLVER cannot be enabled when Z3 is not available.")
  endif()

  set(LLVM_WITH_Z3 1)
endif()

set(LLVM_ENABLE_Z3_SOLVER_DEFAULT "${Z3_FOUND}")


if( LLVM_TARGETS_TO_BUILD STREQUAL "all" )
  set( LLVM_TARGETS_TO_BUILD ${LLVM_ALL_TARGETS} )
endif()

set(LLVM_TARGETS_TO_BUILD
   ${LLVM_TARGETS_TO_BUILD}
   ${LLVM_EXPERIMENTAL_TARGETS_TO_BUILD})
list(REMOVE_DUPLICATES LLVM_TARGETS_TO_BUILD)

option(LLVM_ENABLE_PIC "Build Position-Independent Code" ON)
option(LLVM_ENABLE_MODULES "Compile with C++ modules enabled." OFF)
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  option(LLVM_ENABLE_MODULE_DEBUGGING "Compile with -gmodules." ON)
else()
  option(LLVM_ENABLE_MODULE_DEBUGGING "Compile with -gmodules." OFF)
endif()
option(LLVM_ENABLE_LOCAL_SUBMODULE_VISIBILITY "Compile with -fmodules-local-submodule-visibility." ON)
option(LLVM_ENABLE_LIBCXX "Use libc++ if available." OFF)
option(LLVM_STATIC_LINK_CXX_STDLIB "Statically link the standard library." OFF)
option(LLVM_ENABLE_LLD "Use lld as C and C++ linker." OFF)
option(LLVM_ENABLE_PEDANTIC "Compile with pedantic enabled." ON)
option(LLVM_ENABLE_WERROR "Fail and stop if a warning is triggered." OFF)

option(LLVM_ENABLE_DUMP "Enable dump functions even when assertions are disabled" OFF)
option(LLVM_UNREACHABLE_OPTIMIZE "Optimize llvm_unreachable() as undefined behavior (default), guaranteed trap when OFF" ON)

if( NOT uppercase_CMAKE_BUILD_TYPE STREQUAL "DEBUG" )
  option(LLVM_ENABLE_ASSERTIONS "Enable assertions" OFF)
else()
  option(LLVM_ENABLE_ASSERTIONS "Enable assertions" ON)
endif()

option(LLVM_ENABLE_EXPENSIVE_CHECKS "Enable expensive checks" OFF)

# While adding scalable vector support to LLVM, we temporarily want to
# allow an implicit conversion of TypeSize to uint64_t, and to allow
# code to get the fixed number of elements from a possibly scalable vector.
# This CMake flag enables a more strict mode where it asserts that the type
# is not a scalable vector type.
#
# Enabling this flag makes it easier to find cases where the compiler makes
# assumptions on the size being 'fixed size', when building tests for
# SVE/SVE2 or other scalable vector architectures.
option(LLVM_ENABLE_STRICT_FIXED_SIZE_VECTORS
       "Enable assertions that type is not scalable in implicit conversion from TypeSize to uint64_t and calls to getNumElements" OFF)

set(LLVM_ABI_BREAKING_CHECKS "WITH_ASSERTS" CACHE STRING
  "Enable abi-breaking checks.  Can be WITH_ASSERTS, FORCE_ON or FORCE_OFF.")

option(LLVM_FORCE_USE_OLD_TOOLCHAIN
       "Set to ON to force using an old, unsupported host toolchain." OFF)

set(LLVM_LOCAL_RPATH "" CACHE FILEPATH
  "If set, an absolute path added as rpath on binaries that do not already contain an executable-relative rpath.")

option(LLVM_TEMPORARILY_ALLOW_OLD_TOOLCHAIN
       "Set to ON to only warn when using a toolchain which is about to be deprecated, instead of emitting an error." OFF)
option(LLVM_USE_INTEL_JITEVENTS
  "Use Intel JIT API to inform Intel(R) VTune(TM) Amplifier XE 2011 about JIT code"
  OFF)

if( LLVM_USE_INTEL_JITEVENTS )
  # Verify we are on a supported platform
  if( NOT CMAKE_SYSTEM_NAME MATCHES "Windows" AND NOT CMAKE_SYSTEM_NAME MATCHES "Linux" )
    message(FATAL_ERROR
      "Intel JIT API support is available on Linux and Windows only.")
  endif()
endif( LLVM_USE_INTEL_JITEVENTS )

option(LLVM_USE_OPROFILE
  "Use opagent JIT interface to inform OProfile about JIT code" OFF)

option(LLVM_EXTERNALIZE_DEBUGINFO
  "Generate dSYM files and strip executables and libraries (Darwin Only)" OFF)

set(LLVM_CODESIGNING_IDENTITY "" CACHE STRING
  "Sign executables and dylibs with the given identity or skip if empty (Darwin Only)")

# If enabled, verify we are on a platform that supports oprofile.
if( LLVM_USE_OPROFILE )
  if( NOT CMAKE_SYSTEM_NAME MATCHES "Linux" )
    message(FATAL_ERROR "OProfile support is available on Linux only.")
  endif( NOT CMAKE_SYSTEM_NAME MATCHES "Linux" )
endif( LLVM_USE_OPROFILE )

option(LLVM_USE_PERF
  "Use perf JIT interface to inform perf about JIT code" OFF)

# If enabled, verify we are on a platform that supports perf.
if( LLVM_USE_PERF )
  if( NOT CMAKE_SYSTEM_NAME MATCHES "Linux" )
    message(FATAL_ERROR "perf support is available on Linux only.")
  endif( NOT CMAKE_SYSTEM_NAME MATCHES "Linux" )
endif( LLVM_USE_PERF )

set(LLVM_USE_SANITIZER "" CACHE STRING
  "Define the sanitizer used to build binaries and tests.")
option(LLVM_OPTIMIZE_SANITIZED_BUILDS "Pass -O1 on debug sanitizer builds" ON)
set(LLVM_UBSAN_FLAGS
    "-fsanitize=undefined -fno-sanitize=vptr,function -fno-sanitize-recover=all"
    CACHE STRING
    "Compile flags set to enable UBSan. Only used if LLVM_USE_SANITIZER contains 'Undefined'.")
set(LLVM_LIB_FUZZING_ENGINE "" CACHE PATH
  "Path to fuzzing library for linking with fuzz targets")

option(LLVM_USE_SPLIT_DWARF
  "Use -gsplit-dwarf when compiling llvm and --gdb-index when linking." OFF)

# Define an option controlling whether we should build for 32-bit on 64-bit
# platforms, where supported.
if( CMAKE_SIZEOF_VOID_P EQUAL 8 AND NOT (WIN32 OR ${CMAKE_SYSTEM_NAME} MATCHES "AIX"))
  # TODO: support other platforms and toolchains.
  option(LLVM_BUILD_32_BITS "Build 32 bits executables and libraries." OFF)
endif()

# Define the default arguments to use with 'lit', and an option for the user to
# override.
set(LIT_ARGS_DEFAULT "-sv")
if (MSVC_IDE OR XCODE)
  set(LIT_ARGS_DEFAULT "${LIT_ARGS_DEFAULT} --no-progress-bar")
endif()
set(LLVM_LIT_ARGS "${LIT_ARGS_DEFAULT}" CACHE STRING "Default options for lit")

# On Win32 hosts, provide an option to specify the path to the GnuWin32 tools.
if( WIN32 AND NOT CYGWIN )
  set(LLVM_LIT_TOOLS_DIR "" CACHE PATH "Path to GnuWin32 tools")
endif()

set(LLVM_INTEGRATED_CRT_ALLOC "" CACHE PATH "Replace the Windows CRT allocator with any of {rpmalloc|mimalloc|snmalloc}. Only works with /MT enabled.")
if(LLVM_INTEGRATED_CRT_ALLOC)
  if(NOT WIN32)
    message(FATAL_ERROR "LLVM_INTEGRATED_CRT_ALLOC is only supported on Windows.")
  endif()
  if(LLVM_USE_SANITIZER)
    message(FATAL_ERROR "LLVM_INTEGRATED_CRT_ALLOC cannot be used along with LLVM_USE_SANITIZER!")
  endif()
  if(CMAKE_BUILD_TYPE AND uppercase_CMAKE_BUILD_TYPE STREQUAL "DEBUG")
    message(FATAL_ERROR "The Debug target isn't supported along with LLVM_INTEGRATED_CRT_ALLOC!")
  endif()
endif()

# Define options to control the inclusion and default build behavior for
# components which may not strictly be necessary (tools, examples, and tests).
#
# This is primarily to support building smaller or faster project files.
option(LLVM_INCLUDE_TOOLS "Generate build targets for the LLVM tools." ON)
option(LLVM_BUILD_TOOLS
  "Build the LLVM tools. If OFF, just generate build targets." ON)

option(LLVM_INCLUDE_UTILS "Generate build targets for the LLVM utils." ON)
option(LLVM_BUILD_UTILS
  "Build LLVM utility binaries. If OFF, just generate build targets." ON)

option(LLVM_INCLUDE_RUNTIMES "Generate build targets for the LLVM runtimes." ON)
option(LLVM_BUILD_RUNTIMES
  "Build the LLVM runtimes. If OFF, just generate build targets." ON)

option(LLVM_BUILD_RUNTIME
  "Build the LLVM runtime libraries." ON)
option(LLVM_BUILD_EXAMPLES
  "Build the LLVM example programs. If OFF, just generate build targets." OFF)
option(LLVM_INCLUDE_EXAMPLES "Generate build targets for the LLVM examples" ON)

if(LLVM_BUILD_EXAMPLES)
  add_definitions(-DBUILD_EXAMPLES)
endif(LLVM_BUILD_EXAMPLES)

option(LLVM_BUILD_TESTS
  "Build LLVM unit tests. If OFF, just generate build targets." OFF)
option(LLVM_INCLUDE_TESTS "Generate build targets for the LLVM unit tests." ON)
option(LLVM_INCLUDE_GO_TESTS "Include the Go bindings tests in test build targets." OFF)

option(LLVM_BUILD_BENCHMARKS "Add LLVM benchmark targets to the list of default
targets. If OFF, benchmarks still could be built using Benchmarks target." OFF)
option(LLVM_INCLUDE_BENCHMARKS "Generate benchmark targets. If OFF, benchmarks can't be built." ON)

option (LLVM_BUILD_DOCS "Build the llvm documentation." OFF)
option (LLVM_INCLUDE_DOCS "Generate build targets for llvm documentation." ON)
option (LLVM_ENABLE_DOXYGEN "Use doxygen to generate llvm API documentation." OFF)
option (LLVM_ENABLE_SPHINX "Use Sphinx to generate llvm documentation." OFF)
option (LLVM_ENABLE_OCAMLDOC "Build OCaml bindings documentation." ON)
option (LLVM_ENABLE_BINDINGS "Build bindings." ON)

set(LLVM_INSTALL_DOXYGEN_HTML_DIR "${CMAKE_INSTALL_DOCDIR}/llvm/doxygen-html"
    CACHE STRING "Doxygen-generated HTML documentation install directory")
set(LLVM_INSTALL_OCAMLDOC_HTML_DIR "${CMAKE_INSTALL_DOCDIR}/llvm/ocaml-html"
    CACHE STRING "OCamldoc-generated HTML documentation install directory")

option (LLVM_BUILD_EXTERNAL_COMPILER_RT
  "Build compiler-rt as an external project." OFF)

option (LLVM_VERSION_PRINTER_SHOW_HOST_TARGET_INFO
  "Show target and host info when tools are invoked with --version." ON)

# You can configure which libraries from LLVM you want to include in the
# shared library by setting LLVM_DYLIB_COMPONENTS to a semi-colon delimited
# list of LLVM components. All component names handled by llvm-config are valid.
if(NOT DEFINED LLVM_DYLIB_COMPONENTS)
  set(LLVM_DYLIB_COMPONENTS "all" CACHE STRING
    "Semicolon-separated list of components to include in libLLVM, or \"all\".")
endif()

if(MSVC)
  option(LLVM_BUILD_LLVM_C_DYLIB "Build LLVM-C.dll (Windows only)" ON)
  # Set this variable to OFF here so it can't be set with a command-line
  # argument.
  set (LLVM_LINK_LLVM_DYLIB OFF)
  if (BUILD_SHARED_LIBS)
    message(FATAL_ERROR "BUILD_SHARED_LIBS options is not supported on Windows.")
  endif()
else()
  option(LLVM_LINK_LLVM_DYLIB "Link tools against the libllvm dynamic library" OFF)
  option(LLVM_BUILD_LLVM_C_DYLIB "Build libllvm-c re-export library (Darwin only)" OFF)
  set(LLVM_BUILD_LLVM_DYLIB_default OFF)
  if(LLVM_LINK_LLVM_DYLIB OR LLVM_BUILD_LLVM_C_DYLIB)
    set(LLVM_BUILD_LLVM_DYLIB_default ON)
  endif()
  option(LLVM_BUILD_LLVM_DYLIB "Build libllvm dynamic library" ${LLVM_BUILD_LLVM_DYLIB_default})
endif()

if (LLVM_LINK_LLVM_DYLIB AND BUILD_SHARED_LIBS)
  message(FATAL_ERROR "Cannot enable BUILD_SHARED_LIBS with LLVM_LINK_LLVM_DYLIB.  We recommend disabling BUILD_SHARED_LIBS.")
endif()

option(LLVM_OPTIMIZED_TABLEGEN "Force TableGen to be built with optimization" OFF)
if(CMAKE_CROSSCOMPILING OR (LLVM_OPTIMIZED_TABLEGEN AND (LLVM_ENABLE_ASSERTIONS OR CMAKE_CONFIGURATION_TYPES)))
  set(LLVM_USE_HOST_TOOLS ON)
endif()

option(LLVM_OMIT_DAGISEL_COMMENTS "Do not add comments to DAG ISel" ON)
if (CMAKE_BUILD_TYPE AND uppercase_CMAKE_BUILD_TYPE MATCHES "^(RELWITHDEBINFO|DEBUG)$")
  set(LLVM_OMIT_DAGISEL_COMMENTS OFF)
endif()

if (MSVC_IDE)
  option(LLVM_ADD_NATIVE_VISUALIZERS_TO_SOLUTION "Configure project to use Visual Studio native visualizers" TRUE)
endif()

if (LLVM_BUILD_INSTRUMENTED OR LLVM_BUILD_INSTRUMENTED_COVERAGE OR
    LLVM_ENABLE_IR_PGO)
  if(NOT LLVM_PROFILE_MERGE_POOL_SIZE)
    # A pool size of 1-2 is probably sufficient on a SSD. 3-4 should be fine
    # for spining disks. Anything higher may only help on slower mediums.
    set(LLVM_PROFILE_MERGE_POOL_SIZE "4")
  endif()
  if(NOT LLVM_PROFILE_FILE_PATTERN)
    if(NOT LLVM_PROFILE_DATA_DIR)
      file(TO_NATIVE_PATH "${LLVM_BINARY_DIR}/profiles" LLVM_PROFILE_DATA_DIR)
    endif()
    file(TO_NATIVE_PATH "${LLVM_PROFILE_DATA_DIR}/%${LLVM_PROFILE_MERGE_POOL_SIZE}m.profraw" LLVM_PROFILE_FILE_PATTERN)
  endif()
  if(NOT LLVM_CSPROFILE_FILE_PATTERN)
    if(NOT LLVM_CSPROFILE_DATA_DIR)
      file(TO_NATIVE_PATH "${LLVM_BINARY_DIR}/csprofiles" LLVM_CSPROFILE_DATA_DIR)
    endif()
    file(TO_NATIVE_PATH "${LLVM_CSPROFILE_DATA_DIR}/%${LLVM_PROFILE_MERGE_POOL_SIZE}m.profraw" LLVM_CSPROFILE_FILE_PATTERN)
  endif()
endif()

if (LLVM_BUILD_STATIC)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
  # Remove shared library suffixes from use in find_library
  foreach (shared_lib_suffix ${CMAKE_SHARED_LIBRARY_SUFFIX} ${CMAKE_IMPORT_LIBRARY_SUFFIX})
    list(FIND CMAKE_FIND_LIBRARY_SUFFIXES ${shared_lib_suffix} shared_lib_suffix_idx)
    if(NOT ${shared_lib_suffix_idx} EQUAL -1)
      list(REMOVE_AT CMAKE_FIND_LIBRARY_SUFFIXES ${shared_lib_suffix_idx})
    endif()
  endforeach()
endif()

# Use libtool instead of ar if you are both on an Apple host, and targeting Apple.
if(CMAKE_HOST_APPLE AND APPLE)
  include(UseLibtool)
endif()

# Override the default target with an environment variable named by LLVM_TARGET_TRIPLE_ENV.
set(LLVM_TARGET_TRIPLE_ENV CACHE STRING "The name of environment variable to override default target. Disabled by blank.")
mark_as_advanced(LLVM_TARGET_TRIPLE_ENV)

# Per target dir not yet supported on Arm 32 bit due to arm vs armhf handling
if(CMAKE_SYSTEM_NAME MATCHES "Linux" AND NOT CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
  set(LLVM_ENABLE_PER_TARGET_RUNTIME_DIR_default ON)
else()
  set(LLVM_ENABLE_PER_TARGET_RUNTIME_DIR_default OFF)
endif()
set(LLVM_ENABLE_PER_TARGET_RUNTIME_DIR ${LLVM_ENABLE_PER_TARGET_RUNTIME_DIR_default} CACHE BOOL
  "Enable per-target runtimes directory")

set(LLVM_PROFDATA_FILE "" CACHE FILEPATH
  "Profiling data file to use when compiling in order to improve runtime performance.")

if(LLVM_INCLUDE_TESTS)
  # Lit test suite requires at least python 3.6
  set(LLVM_MINIMUM_PYTHON_VERSION 3.6)
else()
  # FIXME: it is unknown if this is the actual minimum bound
  set(LLVM_MINIMUM_PYTHON_VERSION 3.0)
endif()

# Find python before including config-ix, since it needs to be able to search
# for python modules.
find_package(Python3 ${LLVM_MINIMUM_PYTHON_VERSION} REQUIRED
    COMPONENTS Interpreter)

# All options referred to from HandleLLVMOptions have to be specified
# BEFORE this include, otherwise options will not be correctly set on
# first cmake run
include(config-ix)

# By default, we target the host, but this can be overridden at CMake
# invocation time. Except on 64-bit AIX, where the system toolchain
# expect 32-bit objects by default.
if("${LLVM_HOST_TRIPLE}" MATCHES "^powerpc64-ibm-aix")
  string(REGEX REPLACE "^powerpc64" "powerpc" LLVM_DEFAULT_TARGET_TRIPLE_default "${LLVM_HOST_TRIPLE}")
else()
  set(LLVM_DEFAULT_TARGET_TRIPLE_default "${LLVM_HOST_TRIPLE}")
endif()

set(LLVM_DEFAULT_TARGET_TRIPLE "${LLVM_DEFAULT_TARGET_TRIPLE_default}" CACHE STRING
  "Default target for which LLVM will generate code." )
if (TARGET_TRIPLE)
  message(WARNING "TARGET_TRIPLE is deprecated and will be removed in a future release. "
                  "Please use LLVM_DEFAULT_TARGET_TRIPLE instead.")
  set(LLVM_TARGET_TRIPLE "${TARGET_TRIPLE}")
else()
  set(LLVM_TARGET_TRIPLE "${LLVM_DEFAULT_TARGET_TRIPLE}")
endif()
message(STATUS "LLVM host triple: ${LLVM_HOST_TRIPLE}")
message(STATUS "LLVM default target triple: ${LLVM_DEFAULT_TARGET_TRIPLE}")

# INTEL_CUSTOMIZATION
if (UWD_BUILD)
  add_definitions( -DWINDOWS_ONECORE )
  if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(LLVM_USE_CRT_RELEASE "MT" CACHE STRING "Link rintime library statically" FORCE)
    set(CMAKE_STANDARD_LIBRARIES "OneCore.lib libcmt.lib libvcruntime.lib libucrt.lib"
        CACHE STRING "Building with OneCore.lib" FORCE)
  elseif (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(LLVM_USE_CRT_DEBUG "MTd" CACHE STRING "Link rintime library statically" FORCE)
    set(CMAKE_STANDARD_LIBRARIES "OneCore.lib libcmtd.lib libvcruntimed.lib libucrtd.lib"
        CACHE STRING "Building with OneCore.lib" FORCE)
  endif()
endif()
#end INTEL_CUSTOMIZATION
if(WIN32 OR CYGWIN)
  if(BUILD_SHARED_LIBS OR LLVM_BUILD_LLVM_DYLIB)
    set(LLVM_ENABLE_PLUGINS_default ON)
  else()
    set(LLVM_ENABLE_PLUGINS_default OFF)
  endif()
else()
  set(LLVM_ENABLE_PLUGINS_default ${LLVM_ENABLE_PIC})
endif()
option(LLVM_ENABLE_PLUGINS "Enable plugin support" ${LLVM_ENABLE_PLUGINS_default})

if(INTEL_CUSTOMIZATION)
  set(LLVM_ENABLE_NEW_PASS_MANAGER TRUE CACHE BOOL
      "Enable the new pass manager by default.")
if(NOT LLVM_ENABLE_NEW_PASS_MANAGER)
#  message(FATAL_ERROR "Enabling the legacy pass manager on the cmake level is"
#                      " no longer supported.")
   message(WARNING "Using the legacy pass manager for the optimization pipeline"
                  " is deprecated. The functionality will degrade over time and"
                  " be removed in a future release.")
endif()
endif(INTEL_CUSTOMIZATION)

set(DPCPP_ENABLE_OPAQUE_POINTERS FALSE CACHE BOOL
   "Enable opaque pointers by default.")
if (DPCPP_ENABLE_OPAQUE_POINTERS)
  add_definitions("-DENABLE_OPAQUE_POINTERS=1")
endif(DPCPP_ENABLE_OPAQUE_POINTERS)

include(HandleLLVMOptions)

######

# INTEL_COLLAB
SET(INTEL_COLLAB ON)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DINTEL_COLLAB=1")
# end INTEL_COLLAB
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DINTEL_CUSTOMIZATION=1")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DINTEL_SPECIFIC_OPENMP=1")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DINTEL_SPECIFIC_CILKPLUS=1")
# INTEL_CUSTOMIZATION
# LLVM_INTELFEATURE_PREFIX is a temporary workaround for specifying
# Intel features names as arguments to is_intel_feature_enabled()
# and intel_add_file(), and keeping the feature filtering tool
# from detecting this as illegal usage.  Just prepend the feature
# name with ${LLVM_INTELFEATURE_PREFIX}_, when needed.
SET(LLVM_INTELFEATURE_PREFIX "INTEL_FEATURE")
# end INTEL_CUSTOMIZATION

if(INTEL_CUSTOMIZATION)
  # Set flags for TableGen invocations.  LLVM_INTEL_TABLEGEN_FLAGS
  # must be a list of options, not a string, because of the way
  # tablegen()->add_custom_command() process them.  If the options
  # are space separated, the final build command will escape the
  # spaces.  This is not a problem, but it looks awkward.
  set(LLVM_INTEL_TABLEGEN_FLAGS
    -DINTEL_CUSTOMIZATION
    )

# INTEL_FEATURE_CSA
  if (";${LLVM_TARGETS_TO_BUILD};" MATCHES ";CSA;")
    set(INTEL_ANY_OPTIONAL_COMPONENTS TRUE)
    set(INTEL_CSA_INCLUDE_DIR
        ${LLVM_MAIN_SRC_DIR}/Intel_OptionalComponents/CSA/include)
    include_directories(AFTER ${INTEL_CSA_INCLUDE_DIR})
    SET(LLVMOPTIONALCOMPONENTS ${LLVMOPTIONALCOMPONENTS}
                               Intel_CSATableGen
                               Intel_CSAScalarOpt)
  endif()
# end INTEL_FEATURE_CSA

  # Make sure LLVM_INTEL_FEATURES is in CMake cache, and its default value
  # is "no-features".  Every ICS configuration must explicitly set
  # LLVM_INTEL_FEATURES.  Having LLVM_INTEL_FEATURES in CMake cache
  # also helps to understand with which features the compiler binary
  # was built.  In addition, it allows using CMake's -C option
  # to specify a cache file with default value of LLVM_INTEL_FEATURES,
  # so we may have such a cache file for each ICS configuration instead
  # of explicitly listing features in VRD fils using -intel-features option.
  set(LLVM_INTEL_FEATURES ""
      CACHE STRING "Semicolon-separated list of Intel features to enable.")

  # Read Intel_SupportedFeatures.txt and verify that each feature
  # is declared properly.
  set(supported_features "")
  string(CONCAT LLVM_INTEL_SUPPORTED_FEATURES_FILE
         "${LLVM_MAIN_SRC_DIR}/Intel_OptionalComponents/"
         "Intel_SupportedFeatures.txt")
  if (EXISTS ${LLVM_INTEL_SUPPORTED_FEATURES_FILE})
    file(STRINGS ${LLVM_INTEL_SUPPORTED_FEATURES_FILE} sf)
    foreach(line ${sf})
      # Each feature specification start with "^@".
      string(REGEX MATCH "^@(.*)$" _dummy "${line}")
      if (${CMAKE_MATCH_COUNT} EQUAL 0)
        continue()
      endif()
      # Each feature specification is of the following form:
      #     @<feature name>:<ip classification>@
      # We do ignore any extra text after the terminating '@'.
      string(REGEX MATCH "([^:]+):([^:]+)@" _dummy "${CMAKE_MATCH_1}")
      if (${CMAKE_MATCH_COUNT} EQUAL 0)
        # Unfortunately, we cannot identify the line number.
        message(FATAL_ERROR "Invalid feature specification in "
                            "${LLVM_INTEL_SUPPORTED_FEATURES_FILE}: ${line}")
      endif()

      set(feat ${CMAKE_MATCH_1})
      set(ipclass ${CMAKE_MATCH_2})

      if ((NOT "${ipclass}" STREQUAL "SHARED") AND
          (NOT "${ipclass}" STREQUAL "SECRET"))
        message(FATAL_ERROR "Invalid feature IP class ${ipclass} in:\n${line}\n"
                            "Please use either SHARED or SECRET.")
      endif()
      list(FIND supported_features ${feat} idx)
      if (NOT idx LESS 0)
        message(FATAL_ERROR "Duplicate definition of feature ${feat} "
                            "in ${LLVM_INTEL_SUPPORTED_FEATURES_FILE}")
      endif()
      list(APPEND supported_features ${feat})
    endforeach()
  endif()

  # If a feature (e.g. new ISA) becomes public, we probably want to
  # enable it for all compiler builds.  Use the list below to unconditionally
  # enable such features (until they are upstreamed and the feature controls
  # are removed).
  set(features_enabled_by_default ${LLVM_INTEL_FEATURES})
  # Add each feature enabled by default below
  # WARNING: features added here will be enabled for any compiler build,
  #          so you must not add here any features that must be controlled
  #          depending on the compiler build flavor, e.g. INTEL_INTERNAL_BUILD
  #          must never be enabled for Intel release builds.

# INTEL_FEATURE_SW_ADVANCED
  option(INTEL_ENABLE_SW_ADVANCED "Advanced optimizations not to be shared in
    source form" ON)
  if (INTEL_ENABLE_SW_ADVANCED)
    list(APPEND features_enabled_by_default
       "${LLVM_INTELFEATURE_PREFIX}_SW_ADVANCED"
    )
  endif(INTEL_ENABLE_SW_ADVANCED)
# end INTEL_FEATURE_SW_ADVANCED

# INTEL_FEATURE_SHARED_SW_ADVANCED
  option(INTEL_ENABLE_SHARED_SW_ADVANCED "Advanced optimizations that can
  be shared inside Intel in source form" ON)
  if (INTEL_ENABLE_SHARED_SW_ADVANCED)
    list(APPEND features_enabled_by_default
       "${LLVM_INTELFEATURE_PREFIX}_SHARED_SW_ADVANCED"
    )
  endif(INTEL_ENABLE_SHARED_SW_ADVANCED)
# end INTEL_FEATURE_SHARED_SW_ADVANCED

# INTEL_FEATURE_SW_DTRANS
  option(INTEL_ENABLE_SW_DTRANS "Include DTrans optimizations" ON)
  # Extra boolean variable that will be used for the Python configuration
  # files to identify if DTrans build is enabled
  set (INTEL_DTRANS_EXTRA_OPTS 0)
  if (INTEL_ENABLE_SW_DTRANS)
    list(APPEND features_enabled_by_default
       "${LLVM_INTELFEATURE_PREFIX}_SW_DTRANS"
    )
    # When there are multiple optional components, each of them should
    # set this when included. This causes the Intel_OptionalComponents
    # directory to be added as a subdirectory.
    set(INTEL_ANY_OPTIONAL_COMPONENTS TRUE)
    set(INTEL_DTRANS_INCLUDE_DIR
        ${LLVM_MAIN_SRC_DIR}/Intel_OptionalComponents/DTrans/include)
    include_directories(AFTER ${INTEL_DTRANS_INCLUDE_DIR})
    SET(LLVMOPTIONALCOMPONENTS ${LLVMOPTIONALCOMPONENTS}
                               Intel_DTrans
                               Intel_DTransOpt
                               Intel_DTransAnalysis)
    set (INTEL_DTRANS_EXTRA_OPTS 1)
  endif(INTEL_ENABLE_SW_DTRANS)
# end INTEL_FEATURE_SW_DTRANS

  # Force adding features enabled by default to CMake cache.
  # features_enabled_by_default list now contains all features
  # from the original LLVM_INTEL_FEATURES and also the features
  # added explicitly above.
  if (features_enabled_by_default)
    list(REMOVE_DUPLICATES features_enabled_by_default)

    if (";${features_enabled_by_default};" MATCHES
        ";${LLVM_INTELFEATURE_PREFIX}_XISA;")
      # Expand XISA into a list of new non-public ISA features, and also
      # delete XISA from the list.  Note that XISA is a "macro" feature
      # and it must never be used to guard any code.
      list(REMOVE_ITEM features_enabled_by_default
           "${LLVM_INTELFEATURE_PREFIX}_XISA")

# INTEL_FEATURE_CPU_RPL
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_CPU_RPL)
# end INTEL_FEATURE_CPU_RPL
# INTEL_FEATURE_CPU_GNR
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_CPU_GNR)
# end INTEL_FEATURE_CPU_GNR
# INTEL_FEATURE_CPU_DMR
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_CPU_DMR)
# end INTEL_FEATURE_CPU_DMR

# INTEL_FEATURE_XISA_COMMON
      # This is used to guard code that shared by more then one XISA features.
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_XISA_COMMON)
#end INTEL_FEATURE_XISA_COMMON

# INTEL_FEATURE_ISA_AMX_BF8
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_AMX_BF8)
# end INTEL_FEATURE_ISA_AMX_BF8

# INTEL_FEATURE_ISA_AMX_MEMADVISE
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_AMX_MEMADVISE)
# end INTEL_FEATURE_ISA_AMX_MEMADVISE

# INTEL_FEATURE_ISA_AMX_MEMADVISE_EVEX
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_AMX_MEMADVISE_EVEX)
# end INTEL_FEATURE_ISA_AMX_MEMADVISE_EVEX


# INTEL_FEATURE_ISA_AMX_FUTURE
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_AMX_FUTURE)
# end INTEL_FEATURE_ISA_AMX_FUTURE

# INTEL_FEATURE_ISA_AMX_LNC
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_AMX_LNC)
# end INTEL_FEATURE_ISA_AMX_LNC

# INTEL_FEATURE_ISA_AMX_AVX512
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_AMX_AVX512)
# end INTEL_FEATURE_ISA_AMX_AVX512

# INTEL_FEATURE_ISA_AMX_FP16
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_AMX_FP16)
# end INTEL_FEATURE_ISA_AMX_FP16

# INTEL_FEATURE_ISA_AMX_MEMORY2
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_AMX_MEMORY2)
# end INTEL_FEATURE_ISA_AMX_MEMORY2

# INTEL_FEATURE_ISA_AMX_MEMORY
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_AMX_MEMORY)
# end INTEL_FEATURE_ISA_AMX_MEMORY

# INTEL_FEATURE_ISA_AMX_BF16_EVEX
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_AMX_BF16_EVEX)
# end INTEL_FEATURE_ISA_AMX_BF16_EVEX

# INTEL_FEATURE_ISA_AMX_ELEMENT_EVEX
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_AMX_ELEMENT_EVEX)
# end INTEL_FEATURE_ISA_AMX_ELEMENT_EVEX

# INTEL_FEATURE_ISA_AMX_CONVERT
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_AMX_CONVERT)
# end INTEL_FEATURE_ISA_AMX_CONVERT

# INTEL_FEATURE_ISA_AMX_INT8_EVEX
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_AMX_INT8_EVEX)
# end INTEL_FEATURE_ISA_AMX_INT8_EVEX

# INTEL_FEATURE_ISA_AMX_TILE_EVEX
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_AMX_TILE_EVEX)
# end INTEL_FEATURE_ISA_AMX_TILE_EVEX

# INTEL_FEATURE_ISA_AMX_TILE2
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_AMX_TILE2)
# end INTEL_FEATURE_ISA_AMX_TILE2

# INTEL_FEATURE_ISA_AMX_TRANSPOSE2
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_AMX_TRANSPOSE2)
# end INTEL_FEATURE_ISA_AMX_TRANSPOSE2

# INTEL_FEATURE_ISA_AMX_TRANSPOSE
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_AMX_TRANSPOSE)
# end INTEL_FEATURE_ISA_AMX_TRANSPOSE

# INTEL_FEATURE_ISA_AMX_COMPLEX
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_AMX_COMPLEX)
# end INTEL_FEATURE_ISA_AMX_COMPLEX

# INTEL_FEATURE_ISA_AMX_TF32
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_AMX_TF32)
# end INTEL_FEATURE_ISA_AMX_TF32

# INTEL_FEATURE_ISA_AVX512_VNNI_INT8
list(APPEND features_enabled_by_default
     ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX512_VNNI_INT8)
# end INTEL_FEATURE_ISA_AVX512_VNNI_INT8

# INTEL_FEATURE_ISA_AVX512_VNNI_FP16
list(APPEND features_enabled_by_default
     ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX512_VNNI_FP16)
# end INTEL_FEATURE_ISA_AVX512_VNNI_FP16

# INTEL_FEATURE_ISA_AVX_VNNI_INT8
list(APPEND features_enabled_by_default
     ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX_VNNI_INT8)
# end INTEL_FEATURE_ISA_AVX_VNNI_INT8

# INTEL_FEATURE_ISA_AVX_DOTPROD_PHPS
list(APPEND features_enabled_by_default
     ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX_DOTPROD_PHPS)
# end INTEL_FEATURE_ISA_AVX_DOTPROD_PHPS

# INTEL_FEATURE_ISA_AVX512_CONVERT
    list(APPEND features_enabled_by_default
         ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX512_CONVERT)
# end INTEL_FEATURE_ISA_AVX512_CONVERT

# INTEL_FEATURE_ISA_AVX_CONVERT
list(APPEND features_enabled_by_default
     ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX_CONVERT)
# end INTEL_FEATURE_ISA_AVX_CONVERT

# INTEL_FEATURE_ISA_AVX_BF16
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX_BF16)
# end INTEL_FEATURE_ISA_AVX_BF16

# INTEL_FEATURE_ISA_AVX_IFMA
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX_IFMA)
# end INTEL_FEATURE_ISA_AVX_IFMA

# INTEL_FEATURE_ISA_AVX_COMPRESS
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX_COMPRESS)
# end INTEL_FEATURE_ISA_AVX_COMPRESS

# INTEL_FEATURE_ISA_AVX_MEMADVISE
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX_MEMADVISE)
# end INTEL_FEATURE_ISA_AVX_MEMADVISE

# INTEL_FEATURE_ISA_AVX512_MEDIAX
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX512_MEDIAX)
# end INTEL_FEATURE_ISA_AVX512_MEDIAX
# INTEL_FEATURE_ISA_AVX_MOVGET
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX_MOVGET)
# end INTEL_FEATURE_ISA_AVX_MOVGET
# INTEL_FEATURE_ISA_AVX512_MOVGET
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX512_MOVGET)
# end INTEL_FEATURE_ISA_AVX512_MOVGET

# INTEL_FEATURE_ISA_GPR_MOVGET
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_GPR_MOVGET)
# end INTEL_FEATURE_ISA_GPR_MOVGET
# INTEL_FEATURE_ISA_MOVGET64B
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_MOVGET64B)
# end INTEL_FEATURE_ISA_MOVGET64B

# INTEL_FEATURE_ESIMD_EMBARGO
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ESIMD_EMBARGO)
# end INTEL_FEATURE_ESIMD_EMBARGO

# INTEL_FEATURE_ISA_RAO_INT
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_RAO_INT)
# end INTEL_FEATURE_ISA_RAO_INT
# INTEL_FEATURE_ISA_AVX_RAO_INT
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX_RAO_INT)
# end INTEL_FEATURE_ISA_AVX_RAO_INT
# INTEL_FEATURE_ISA_AVX_RAO_FP
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX_RAO_FP)
# end INTEL_FEATURE_ISA_AVX_RAO_FP
# INTEL_FEATURE_ISA_AVX512_RAO_INT
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX512_RAO_INT)
# end INTEL_FEATURE_ISA_AVX512_RAO_INT
# INTEL_FEATURE_ISA_AVX512_RAO_FP
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX512_RAO_FP)
# end INTEL_FEATURE_ISA_AVX512_RAO_FP
# INTEL_FEATURE_ISA_AMX_AVX512_CVTROW
# AUTO GENERATED BY TOOL
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_AMX_AVX512_CVTROW)
# end AUTO GENERATED BY TOOL
# end INTEL_FEATURE_ISA_AMX_AVX512_CVTROW
# INTEL_FEATURE_ISA_AVX_NE_CONVERT
#AUTO GENERATED BY TOOL
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX_NE_CONVERT)
# end AUTO GENERATED BY TOOL
# end INTEL_FEATURE_ISA_AVX_NE_CONVERT
# INTEL_FEATURE_ISA_AVX512_NE_CONVERT
#AUTO GENERATED BY TOOL
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX512_NE_CONVERT)
# end AUTO GENERATED BY TOOL
# end INTEL_FEATURE_ISA_AVX512_NE_CONVERT
# INTEL_FEATURE_ISA_SHA512
# AUTO GENERATED BY TOOL
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_SHA512)
# end AUTO GENERATED BY TOOL
# end INTEL_FEATURE_ISA_SHA512
# INTEL_FEATURE_ISA_SM3
# AUTO GENERATED BY TOOL
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_SM3)
# end AUTO GENERATED BY TOOL
# end INTEL_FEATURE_ISA_SM3
# INTEL_FEATURE_ISA_SM4
# AUTO GENERATED BY TOOL
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_SM4)
# end AUTO GENERATED BY TOOL
# end INTEL_FEATURE_ISA_SM4
# INTEL_FEATURE_ISA_DSPV1
# AUTO GENERATED BY TOOL
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_DSPV1)
# end AUTO GENERATED BY TOOL
# end INTEL_FEATURE_ISA_DSPV1
# INTEL_FEATURE_ISA_AVX256
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX256)
# end INTEL_FEATURE_ISA_AVX256
# INTEL_FEATURE_ISA_AVX256P
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX256P)
# end INTEL_FEATURE_ISA_AVX256P
# INTEL_FEATURE_ISA_AVX_VNNI_INT16
# AUTO GENERATED BY TOOL
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX_VNNI_INT16)
# end AUTO GENERATED BY TOOL
# end INTEL_FEATURE_ISA_AVX_VNNI_INT16
# INTEL_FEATURE_ISA_AVX512_VNNI_INT16
# AUTO GENERATED BY TOOL
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX512_VNNI_INT16)
# end AUTO GENERATED BY TOOL
# end INTEL_FEATURE_ISA_AVX512_VNNI_INT16
# INTEL_FEATURE_ISA_AMX_SPARSE
# AUTO GENERATED BY TOOL
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_AMX_SPARSE)
# end AUTO GENERATED BY TOOL
# end INTEL_FEATURE_ISA_AMX_SPARSE
# INTEL_FEATURE_ISA_AMX_V3
# AUTO GENERATED BY TOOL
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_AMX_V3)
# end AUTO GENERATED BY TOOL
# end INTEL_FEATURE_ISA_AMX_V3
# INTEL_FEATURE_ISA_VPINSR_VPEXTR
      list(APPEND features_enabled_by_default
          ${LLVM_INTELFEATURE_PREFIX}_ISA_VPINSR_VPEXTR)
#end INTEL_FEATURE_ISA_VPINSR_VPEXTR
# INTEL_FEATURE_ISA_CMPCCXADD
# AUTO GENERATED BY TOOL
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_CMPCCXADD)
# end AUTO GENERATED BY TOOL
# end INTEL_FEATURE_ISA_CMPCCXADD
# INTEL_FEATURE_ISA_PREFETCHST2
      list(APPEND features_enabled_by_default
          ${LLVM_INTELFEATURE_PREFIX}_ISA_PREFETCHST2)
# end INTEL_FEATURE_ISA_PREFETCHST2
# INTEL_FEATURE_ISA_PREFETCHI
      list(APPEND features_enabled_by_default
          ${LLVM_INTELFEATURE_PREFIX}_ISA_PREFETCHI)
# end INTEL_FEATURE_ISA_PREFETCHI
# INTEL_FEATURE_ISA_AVX512_SAT_CVT
# AUTO GENERATED BY TOOL
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX512_SAT_CVT)
# end AUTO GENERATED BY TOOL
# end INTEL_FEATURE_ISA_AVX512_SAT_CVT
# INTEL_FEATURE_ISA_BF16_BASE
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_BF16_BASE)
# end INTEL_FEATURE_ISA_BF16_BASE
# INTEL_FEATURE_ISA_AVX512_MINMAX
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX512_MINMAX)
# end INTEL_FEATURE_ISA_AVX512_MINMAX
# INTEL_FEATURE_ISA_AVX512_COMPLEX
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX512_COMPLEX)
# end INTEL_FEATURE_ISA_AVX512_COMPLEX
# INTEL_FEATURE_ISA_AVX512_REDUCTION
      list(APPEND features_enabled_by_default
           ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX512_REDUCTION)
# end INTEL_FEATURE_ISA_AVX512_REDUCTION
# INTEL_FEATURE_ISA_AVX512_BF16_NE
      list(APPEND features_enabled_by_default
          ${LLVM_INTELFEATURE_PREFIX}_ISA_AVX512_BF16_NE)
# end INTEL_FEATURE_ISA_AVX512_BF16_NE
# INTEL_FEATURE_ISA_PBNDKB
      list(APPEND features_enabled_by_default
          ${LLVM_INTELFEATURE_PREFIX}_ISA_PBNDKB)
# end INTEL_FEATURE_ISA_PBNDKB
# INTEL_FEATURE_ISA_WRMSRNS
      list(APPEND features_enabled_by_default
          ${LLVM_INTELFEATURE_PREFIX}_ISA_WRMSRNS)
# end INTEL_FEATURE_ISA_WRMSRNS
# INTEL_FEATURE_ISA_RDMSRLIST_WRMSRLIST
      list(APPEND features_enabled_by_default
          ${LLVM_INTELFEATURE_PREFIX}_ISA_RDMSRLIST_WRMSRLIST)
# end INTEL_FEATURE_ISA_RDMSRLIST_WRMSRLIST
    endif()

    set(LLVM_INTEL_FEATURES "${features_enabled_by_default}"
        CACHE STRING
        "Semicolon-separated list of Intel features to enable."
        FORCE)
  endif()

  # If any Intel features are specified for the build, we have to check
  # them against the list of features located in Intel_SupportedFeatures.txt
  list(LENGTH LLVM_INTEL_FEATURES n)
  if (n GREATER 0)
    if (NOT EXISTS ${LLVM_INTEL_SUPPORTED_FEATURES_FILE})
      message(FATAL_ERROR
              "${LLVM_INTEL_SUPPORTED_FEATURES_FILE} does not exist")
    endif()
    # For each feature 'f' enabled for the compiler build check
    # if the feature is present in the 'supported_features' list.  As usual,
    # we have to add semicolons around 'f' to match it inside the list.
    # We also need to add semicolons around the list itself, so that
    # the first starts with semicolon, and the last line ends with a semicolon.
    foreach(f ${LLVM_INTEL_FEATURES})
      # Temporary workaround for the current feature filtering tool,
      # which does not allow arbitrary usage of INTEL_FEATURE\_ prefix.
      # Thus, we cannot declare features with this prefix in
      # Intel_SupportedFeatures.txt.  At the same time, the feature names
      # in LLVM_INTEL_FEATURES are prefixed.  Here, we strip the prefix
      # from the feature name before looking for it in the file.
      string(CONCAT prefix "INTEL_FEATURE" "_")
      string(REPLACE ${prefix} "" fname "${f}")
      string(REGEX MATCH ";${fname};" m ";${supported_features};")
      # If the enabled feature is not present in the features file, then
      # we must report an error to the user.  The result of the regex match
      # is in 'm' - it is an empty string, if there is no match.
      if ("${m}" STREQUAL "")
        message(FATAL_ERROR
            "Intel feature '${f}' is not supported.  "
            "Please add it to ${LLVM_INTEL_SUPPORTED_FEATURES_FILE} "
            "(without ${prefix} prefix).")
      endif()
    endforeach(f)

    # Add dependency to the features file, so that cmake reconfigures
    # itself, once the file changes.  This prevents successful builds,
    # if somebody removed a feature from the features file and does
    # unclean build.  This method of setting the dependency
    # is taken from "${LLVM_BINARY_DIR}/LLVMBuild.cmake" (autogenerated
    # by llvm-build).
    configure_file(${LLVM_INTEL_SUPPORTED_FEATURES_FILE}
                   ${CMAKE_CURRENT_BINARY_DIR}/DummyConfigureOutput)

    # Define feature specific macros.
    foreach(f ${LLVM_INTEL_FEATURES})
      list(APPEND LLVM_INTEL_TABLEGEN_FLAGS "-D${f}")
      add_definitions(-D${f}=1)
    endforeach(f)
  endif()

  # Add knob to enable/disable Protobuf-based binary opt-report feature during
  # compiler's build. Feature is currently enabled by default only for
  # Linux/Unix environment. It is also disabled for sanitizer builds.
  if (WIN32 OR APPLE OR LLVM_USE_SANITIZER)
    option(INTEL_ENABLE_PROTO_BIN_OPTRPT
           "Enable Protobuf-based binary opt-report feature" OFF)
  else()
    option(INTEL_ENABLE_PROTO_BIN_OPTRPT
           "Enable Protobuf-based binary opt-report feature" ON)
  endif()
endif(INTEL_CUSTOMIZATION)

# Configure all of the various header file fragments LLVM uses which depend on
# configuration variables.
set(LLVM_ENUM_TARGETS "")
set(LLVM_ENUM_ASM_PRINTERS "")
set(LLVM_ENUM_ASM_PARSERS "")
set(LLVM_ENUM_DISASSEMBLERS "")
set(LLVM_ENUM_TARGETMCAS "")
foreach(t ${LLVM_TARGETS_TO_BUILD})
  set( td ${LLVM_MAIN_SRC_DIR}/lib/Target/${t} )
  if(INTEL_CUSTOMIZATION)
# INTEL_FEATURE_CSA
    if("${t}" STREQUAL "CSA")
      set(td ${LLVM_MAIN_SRC_DIR}/lib/Target/Intel_${t})
    endif()
# end INTEL_FEATURE_CSA
  endif(INTEL_CUSTOMIZATION)

  list(FIND LLVM_ALL_TARGETS ${t} idx)
  list(FIND LLVM_EXPERIMENTAL_TARGETS_TO_BUILD ${t} idy)
  # At this point, LLVMBUILDTOOL already checked all the targets passed in
  # LLVM_TARGETS_TO_BUILD and LLVM_EXPERIMENTAL_TARGETS_TO_BUILD, so
  # this test just makes sure that any experimental targets were passed via
  # LLVM_EXPERIMENTAL_TARGETS_TO_BUILD, not LLVM_TARGETS_TO_BUILD.
  if( idx LESS 0 AND idy LESS 0 )
    message(FATAL_ERROR "The target `${t}' is experimental and must be passed "
      "via LLVM_EXPERIMENTAL_TARGETS_TO_BUILD.")
  else()
    set(LLVM_ENUM_TARGETS "${LLVM_ENUM_TARGETS}LLVM_TARGET(${t})\n")
  endif()

  file(GLOB asmp_file "${td}/*AsmPrinter.cpp")
  if( asmp_file )
    set(LLVM_ENUM_ASM_PRINTERS
      "${LLVM_ENUM_ASM_PRINTERS}LLVM_ASM_PRINTER(${t})\n")
  endif()
  if( EXISTS ${td}/AsmParser/CMakeLists.txt )
    set(LLVM_ENUM_ASM_PARSERS
      "${LLVM_ENUM_ASM_PARSERS}LLVM_ASM_PARSER(${t})\n")
  endif()
  if( EXISTS ${td}/Disassembler/CMakeLists.txt )
    set(LLVM_ENUM_DISASSEMBLERS
      "${LLVM_ENUM_DISASSEMBLERS}LLVM_DISASSEMBLER(${t})\n")
  endif()
    if( EXISTS ${td}/MCA/CMakeLists.txt )
    set(LLVM_ENUM_TARGETMCAS
      "${LLVM_ENUM_TARGETMCAS}LLVM_TARGETMCA(${t})\n")
  endif()
endforeach(t)

# Provide an LLVM_ namespaced alias for use in #cmakedefine.
set(LLVM_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})

# Produce the target definition files, which provide a way for clients to easily
# include various classes of targets.
configure_file(
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Config/AsmPrinters.def.in
  ${LLVM_INCLUDE_DIR}/llvm/Config/AsmPrinters.def
  )
configure_file(
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Config/AsmParsers.def.in
  ${LLVM_INCLUDE_DIR}/llvm/Config/AsmParsers.def
  )
configure_file(
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Config/Disassemblers.def.in
  ${LLVM_INCLUDE_DIR}/llvm/Config/Disassemblers.def
  )
configure_file(
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Config/Targets.def.in
  ${LLVM_INCLUDE_DIR}/llvm/Config/Targets.def
  )
configure_file(
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Config/TargetMCAs.def.in
  ${LLVM_INCLUDE_DIR}/llvm/Config/TargetMCAs.def
  )

# They are not referenced. See set_output_directory().
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${LLVM_BINARY_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LLVM_BINARY_DIR}/lib${LLVM_LIBDIR_SUFFIX} )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LLVM_BINARY_DIR}/lib${LLVM_LIBDIR_SUFFIX} )

if(LLVM_INCLUDE_TESTS)
  include(GetErrcMessages)
  get_errc_messages(LLVM_LIT_ERRC_MESSAGES)
endif()

# For up-to-date instructions for installing the Tensorflow dependency, refer to
# the bot setup script: https://github.com/google/ml-compiler-opt/blob/main/buildbot/buildbot_init.sh
# In this case, the latest C API library is available for download from
# https://www.tensorflow.org/install/lang_c.
# We will expose the conditional compilation variable,
# LLVM_HAVE_TF_API, through llvm-config.h, so that a user of the LLVM library may
# also leverage the dependency.
set(TENSORFLOW_C_LIB_PATH "" CACHE PATH "Path to TensorFlow C library install")
if (TENSORFLOW_C_LIB_PATH)
  find_library(tensorflow_c_api tensorflow PATHS ${TENSORFLOW_C_LIB_PATH}/lib NO_DEFAULT_PATH REQUIRED)
  # Currently, the protobuf headers are distributed with the pip package that corresponds to the version
  # of the C API library.
  find_library(tensorflow_fx tensorflow_framework PATHS ${TENSORFLOW_C_LIB_PATH}/lib NO_DEFAULT_PATH REQUIRED)
  set(LLVM_HAVE_TF_API "ON" CACHE BOOL "Full Tensorflow API available")
  include_directories(${TENSORFLOW_C_LIB_PATH}/include)
  if (NOT TF_PROTO_HEADERS)
    message(STATUS "TF_PROTO_HEADERS not defined. Looking for tensorflow pip package.")
    execute_process(COMMAND
      ${Python3_EXECUTABLE} "-m" "pip" "show" "tensorflow"
      OUTPUT_VARIABLE TF_PIP_OUT)
    if ("${TF_PIP_OUT}" STREQUAL "")
      message(FATAL ERROR "Tensorflow pip package is also required for 'development' mode (protobuf headers)")
    endif()
    string(REGEX MATCH "Location: ([^\n]*\n)" TF_PIP_LOC "${TF_PIP_OUT}")
    string(REPLACE "Location: " "" TF_PIP ${TF_PIP_LOC})
    string(STRIP ${TF_PIP} TF_PIP)
    set(TF_PROTO_HEADERS "${TF_PIP}/tensorflow/include")
  endif()
  message(STATUS "Using Tensorflow headers under: ${TF_PROTO_HEADERS}")
  include_directories(${TF_PROTO_HEADERS})
  add_definitions("-DGOOGLE_PROTOBUF_NO_RTTI")
  add_definitions("-D_GLIBCXX_USE_CXX11_ABI=0")
endif()

# For up-to-date instructions for installing the Tensorflow dependency, refer to
# the bot setup script: https://github.com/google/ml-compiler-opt/blob/main/buildbot/buildbot_init.sh
# Specifically, assuming python3 is installed:
# python3 -m pip install --upgrade pip && python3 -m pip install --user tf_nightly==2.3.0.dev20200528
# Then set TENSORFLOW_AOT_PATH to the package install - usually it's ~/.local/lib/python3.7/site-packages/tensorflow
#
set(TENSORFLOW_AOT_PATH "" CACHE PATH "Path to TensorFlow pip install dir")

if (NOT TENSORFLOW_AOT_PATH STREQUAL "")
  set(LLVM_HAVE_TF_AOT "ON" CACHE BOOL "Tensorflow AOT available")
  set(TENSORFLOW_AOT_COMPILER
    "${TENSORFLOW_AOT_PATH}/../../../../bin/saved_model_cli"
    CACHE PATH "Path to the Tensorflow AOT compiler")
  include_directories(${TENSORFLOW_AOT_PATH}/include)
  add_subdirectory(${TENSORFLOW_AOT_PATH}/xla_aot_runtime_src
    ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/tf_runtime)
  install(TARGETS tf_xla_runtime EXPORT LLVMExports
    ARCHIVE DESTINATION lib${LLVM_LIBDIR_SUFFIX} COMPONENT tf_xla_runtime)
  set_property(GLOBAL APPEND PROPERTY LLVM_EXPORTS tf_xla_runtime)
  # Once we add more modules, we should handle this more automatically.
  if (DEFINED LLVM_OVERRIDE_MODEL_HEADER_INLINERSIZEMODEL)
    set(LLVM_INLINER_MODEL_PATH "none")
  elseif(NOT DEFINED LLVM_INLINER_MODEL_PATH
      OR "${LLVM_INLINER_MODEL_PATH}" STREQUAL ""
      OR "${LLVM_INLINER_MODEL_PATH}" STREQUAL "autogenerate")
    set(LLVM_INLINER_MODEL_PATH "autogenerate")
    set(LLVM_INLINER_MODEL_AUTOGENERATED 1)
  endif()
  if (DEFINED LLVM_OVERRIDE_MODEL_HEADER_REGALLOCEVICTMODEL)
    set(LLVM_RAEVICT_MODEL_PATH "none")
  elseif(NOT DEFINED LLVM_RAEVICT_MODEL_PATH
      OR "${LLVM_RAEVICT_MODEL_PATH}" STREQUAL ""
      OR "${LLVM_RAEVICT_MODEL_PATH}" STREQUAL "autogenerate")
    set(LLVM_RAEVICT_MODEL_PATH "autogenerate")
    set(LLVM_RAEVICT_MODEL_AUTOGENERATED 1)
  endif()

endif()

# Configure the three LLVM configuration header files.
configure_file(
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Config/config.h.cmake
  ${LLVM_INCLUDE_DIR}/llvm/Config/config.h)
configure_file(
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Config/llvm-config.h.cmake
  ${LLVM_INCLUDE_DIR}/llvm/Config/llvm-config.h)
configure_file(
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Config/abi-breaking.h.cmake
  ${LLVM_INCLUDE_DIR}/llvm/Config/abi-breaking.h)

# Add target for generating source rpm package.
set(LLVM_SRPM_USER_BINARY_SPECFILE ${CMAKE_CURRENT_SOURCE_DIR}/llvm.spec.in
    CACHE FILEPATH ".spec file to use for srpm generation")
set(LLVM_SRPM_BINARY_SPECFILE ${CMAKE_CURRENT_BINARY_DIR}/llvm.spec)
set(LLVM_SRPM_DIR "${CMAKE_CURRENT_BINARY_DIR}/srpm")

get_source_info(${CMAKE_CURRENT_SOURCE_DIR} revision repository)
string(LENGTH "${revision}" revision_length)
set(LLVM_RPM_SPEC_REVISION "${revision}")

configure_file(
  ${LLVM_SRPM_USER_BINARY_SPECFILE}
  ${LLVM_SRPM_BINARY_SPECFILE} @ONLY)

add_custom_target(srpm
  COMMAND cpack -G TGZ --config CPackSourceConfig.cmake -B ${LLVM_SRPM_DIR}/SOURCES
  COMMAND rpmbuild -bs --define '_topdir ${LLVM_SRPM_DIR}' ${LLVM_SRPM_BINARY_SPECFILE})
set_target_properties(srpm PROPERTIES FOLDER "Misc")

if(APPLE AND DARWIN_LTO_LIBRARY)
  set(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} -Wl,-lto_library -Wl,${DARWIN_LTO_LIBRARY}")
  set(CMAKE_SHARED_LINKER_FLAGS
    "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-lto_library -Wl,${DARWIN_LTO_LIBRARY}")
  set(CMAKE_MODULE_LINKER_FLAGS
    "${CMAKE_MODULE_LINKER_FLAGS} -Wl,-lto_library -Wl,${DARWIN_LTO_LIBRARY}")
endif()

# Build with _XOPEN_SOURCE on AIX, as stray macros in _ALL_SOURCE mode tend to
# break things. In this case we need to enable the large-file API as well.
if (UNIX AND ${CMAKE_SYSTEM_NAME} MATCHES "AIX")
          add_definitions("-D_XOPEN_SOURCE=700")
          add_definitions("-D_LARGE_FILE_API")

  # CMake versions less than 3.16 set default linker flags to include -brtl, as
  # well as setting -G when building libraries, so clear them out. Note we only
  # try to clear the form that CMake will set as part of its initial
  # configuration, it is still possible the user may force it as part of a
  # compound option.
  if(CMAKE_VERSION VERSION_LESS 3.16)
    string(REGEX REPLACE "(^|[ \t]+)-Wl,-brtl([ \t]+|$)" " " CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS}")
    string(REGEX REPLACE "(^|[ \t]+)-Wl,-brtl([ \t]+|$)" " " CMAKE_SHARED_LINKER_FLAGS  "${CMAKE_SHARED_LINKER_FLAGS}")
    string(REGEX REPLACE "(^|[ \t]+)-Wl,-brtl([ \t]+|$)" " " CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS}")
    string(REGEX REPLACE "(^|[ \t]+)(-Wl,)?-G([ \t]+|$)" " " CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS
      "${CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS}")
    string(REGEX REPLACE "(^|[ \t]+)(-Wl,)?-G([ \t]+|$)" " " CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS
      "${CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS}")
    string(REGEX REPLACE "(^|[ \t]+)(-Wl,)?-G([ \t]+|$)" " " CMAKE_SHARED_LIBRARY_CREATE_ASM_FLAGS
      "${CMAKE_SHARED_LIBRARY_CREATE_ASM_FLAGS}")
    string(REGEX REPLACE "(^|[ \t]+)-Wl,-G," " -Wl," CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS
      "${CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS}")
    string(REGEX REPLACE "(^|[ \t]+)-Wl,-G," " -Wl," CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS
      "${CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS}")
    string(REGEX REPLACE "(^|[ \t]+)-Wl,-G," " -Wl," CMAKE_SHARED_LIBRARY_CREATE_ASM_FLAGS
      "${CMAKE_SHARED_LIBRARY_CREATE_ASM_FLAGS}")
  endif()

  # Modules should be built with -shared -Wl,-G, so we can use runtime linking
  # with plugins.
  string(APPEND CMAKE_MODULE_LINKER_FLAGS " -shared -Wl,-G")

  # Also set the correct flags for building shared libraries.
  string(APPEND CMAKE_SHARED_LINKER_FLAGS " -shared")
endif()

# Build with _XOPEN_SOURCE on z/OS.
if (CMAKE_SYSTEM_NAME MATCHES "OS390")
  add_definitions("-D_XOPEN_SOURCE=600")
  add_definitions("-D_OPEN_SYS") # Needed for process information.
  add_definitions("-D_OPEN_SYS_FILE_EXT") # Needed for EBCDIC I/O.
endif()

# Build with _FILE_OFFSET_BITS=64 on Solaris to match g++ >= 9.
if (UNIX AND ${CMAKE_SYSTEM_NAME} MATCHES "SunOS")
          add_definitions("-D_FILE_OFFSET_BITS=64")
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories( ${LLVM_INCLUDE_DIR} ${LLVM_MAIN_INCLUDE_DIR})

# when crosscompiling import the executable targets from a file
if(LLVM_USE_HOST_TOOLS)
  include(CrossCompile)
  llvm_create_cross_target(LLVM NATIVE "" Release)
endif(LLVM_USE_HOST_TOOLS)
if(LLVM_TARGET_IS_CROSSCOMPILE_HOST)
# Dummy use to avoid CMake Warning: Manually-specified variables were not used
# (this is a variable that CrossCompile sets on recursive invocations)
endif()

if( ${CMAKE_SYSTEM_NAME} MATCHES SunOS )
   # special hack for Solaris to handle crazy system sys/regset.h
   include_directories("${LLVM_MAIN_INCLUDE_DIR}/llvm/Support/Solaris")
endif( ${CMAKE_SYSTEM_NAME} MATCHES SunOS )

# Make sure we don't get -rdynamic in every binary. For those that need it,
# use export_executable_symbols(target).
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

set(LLVM_EXTRACT_SYMBOLS_FLAGS ""
  CACHE STRING "Additional options to pass to llvm/utils/extract_symbols.py.
  These cannot override the options set by cmake, but can add extra options
  such as --tools.")

include(AddLLVM)
include(TableGen)

include(LLVMDistributionSupport)

if( MINGW AND NOT "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" )
  # People report that -O3 is unreliable on MinGW. The traditional
  # build also uses -O2 for that reason:
  llvm_replace_compiler_option(CMAKE_CXX_FLAGS_RELEASE "-O3" "-O2")
endif()

if(LLVM_INCLUDE_TESTS)
  umbrella_lit_testsuite_begin(check-all)
endif()

# Put this before tblgen. Else we have a circular dependence.
add_subdirectory(lib/Demangle)
add_subdirectory(lib/Support)
add_subdirectory(lib/TableGen)

add_subdirectory(utils/TableGen)

add_subdirectory(include/llvm)

add_subdirectory(lib)

if( LLVM_INCLUDE_UTILS )
  add_subdirectory(utils/FileCheck)
  add_subdirectory(utils/PerfectShuffle)
  add_subdirectory(utils/count)
  add_subdirectory(utils/not)
  add_subdirectory(utils/UnicodeData)
  add_subdirectory(utils/yaml-bench)
  add_subdirectory(utils/split-file)
  if( LLVM_INCLUDE_TESTS )
    add_subdirectory(utils/unittest)
  endif()
else()
  if ( LLVM_INCLUDE_TESTS )
    message(FATAL_ERROR "Including tests when not building utils will not work.
    Either set LLVM_INCLUDE_UTILS to On, or set LLVM_INCLUDE_TESTS to Off.")
  endif()
endif()

# Use LLVM_ADD_NATIVE_VISUALIZERS_TO_SOLUTION instead of LLVM_INCLUDE_UTILS because it is not really a util
if (LLVM_ADD_NATIVE_VISUALIZERS_TO_SOLUTION)
  add_subdirectory(utils/LLVMVisualizers)
endif()

foreach( binding ${LLVM_BINDINGS_LIST} )
  if( EXISTS "${LLVM_MAIN_SRC_DIR}/bindings/${binding}/CMakeLists.txt" )
    add_subdirectory(bindings/${binding})
  endif()
endforeach()

add_subdirectory(projects)

if( LLVM_INCLUDE_TOOLS )
  add_subdirectory(tools)
endif()

if( LLVM_INCLUDE_RUNTIMES )
  add_subdirectory(runtimes)
endif()

if( LLVM_INCLUDE_EXAMPLES )
  add_subdirectory(examples)
endif()

if (INTEL_CUSTOMIZATION)
  if (INTEL_ANY_OPTIONAL_COMPONENTS)
    add_subdirectory(Intel_OptionalComponents)
  endif()
endif (INTEL_CUSTOMIZATION)

if( LLVM_INCLUDE_TESTS )
  if(EXISTS ${LLVM_MAIN_SRC_DIR}/projects/test-suite AND TARGET clang)
    include(LLVMExternalProjectUtils)
    llvm_ExternalProject_Add(test-suite ${LLVM_MAIN_SRC_DIR}/projects/test-suite
      USE_TOOLCHAIN
      EXCLUDE_FROM_ALL
      NO_INSTALL
      ALWAYS_CLEAN)
  endif()
  add_subdirectory(utils/lit)
  add_subdirectory(test)
  add_subdirectory(unittests)

  if (WIN32)
    # This utility is used to prevent crashing tests from calling Dr. Watson on
    # Windows.
    add_subdirectory(utils/KillTheDoctor)
  endif()

  umbrella_lit_testsuite_end(check-all)
  get_property(LLVM_ALL_LIT_DEPENDS GLOBAL PROPERTY LLVM_ALL_LIT_DEPENDS)
  get_property(LLVM_ALL_ADDITIONAL_TEST_DEPENDS
      GLOBAL PROPERTY LLVM_ALL_ADDITIONAL_TEST_DEPENDS)
  add_custom_target(test-depends
      DEPENDS ${LLVM_ALL_LIT_DEPENDS} ${LLVM_ALL_ADDITIONAL_TEST_DEPENDS})
  set_target_properties(test-depends PROPERTIES FOLDER "Tests")
endif()

if (LLVM_INCLUDE_DOCS)
  add_subdirectory(docs)
endif()

add_subdirectory(cmake/modules)

# Do this last so that all lit targets have already been created.
if (LLVM_INCLUDE_UTILS)
  add_subdirectory(utils/llvm-lit)
endif()

if (NOT LLVM_INSTALL_TOOLCHAIN_ONLY)
  install(DIRECTORY include/llvm include/llvm-c
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    COMPONENT llvm-headers
    FILES_MATCHING
    PATTERN "*.def"
    PATTERN "*.h"
    PATTERN "*.td"
    PATTERN "*.inc"
    PATTERN "LICENSE.TXT"
    )

  install(DIRECTORY ${LLVM_INCLUDE_DIR}/llvm ${LLVM_INCLUDE_DIR}/llvm-c
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    COMPONENT llvm-headers
    FILES_MATCHING
    PATTERN "*.def"
    PATTERN "*.h"
    PATTERN "*.gen"
    PATTERN "*.inc"
    # Exclude include/llvm/CMakeFiles/intrinsics_gen.dir, matched by "*.def"
    PATTERN "CMakeFiles" EXCLUDE
    PATTERN "config.h" EXCLUDE
    )

  if (LLVM_INSTALL_MODULEMAPS)
    install(DIRECTORY include/llvm include/llvm-c
            DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
            COMPONENT llvm-headers
            FILES_MATCHING
            PATTERN "module.modulemap"
            )
    install(FILES include/llvm/module.install.modulemap
            DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/llvm"
            COMPONENT llvm-headers
            RENAME "module.extern.modulemap"
            )
  endif(LLVM_INSTALL_MODULEMAPS)

  # Installing the headers needs to depend on generating any public
  # tablegen'd headers.
  add_custom_target(llvm-headers DEPENDS intrinsics_gen omp_gen)
  set_target_properties(llvm-headers PROPERTIES FOLDER "Misc")

  if (NOT LLVM_ENABLE_IDE)
    add_llvm_install_targets(install-llvm-headers
                             DEPENDS llvm-headers
                             COMPONENT llvm-headers)
  endif()

  # Custom target to install all libraries.
  add_custom_target(llvm-libraries)
  set_target_properties(llvm-libraries PROPERTIES FOLDER "Misc")

  if (NOT LLVM_ENABLE_IDE)
    add_llvm_install_targets(install-llvm-libraries
                             DEPENDS llvm-libraries
                             COMPONENT llvm-libraries)
  endif()

  get_property(LLVM_LIBS GLOBAL PROPERTY LLVM_LIBS)
  if(LLVM_LIBS)
    list(REMOVE_DUPLICATES LLVM_LIBS)
    foreach(lib ${LLVM_LIBS})
      add_dependencies(llvm-libraries ${lib})
      if (NOT LLVM_ENABLE_IDE)
        add_dependencies(install-llvm-libraries install-${lib})
        add_dependencies(install-llvm-libraries-stripped install-${lib}-stripped)
      endif()
    endforeach()
  endif()
endif()

# This must be at the end of the LLVM root CMakeLists file because it must run
# after all targets are created.
llvm_distribution_add_targets()
process_llvm_pass_plugins(GEN_CONFIG)
include(CoverageReport)

# This allows us to deploy the Universal CRT DLLs by passing -DCMAKE_INSTALL_UCRT_LIBRARIES=ON to CMake
if (MSVC AND CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows" AND CMAKE_INSTALL_UCRT_LIBRARIES)
  include(InstallRequiredSystemLibraries)
endif()

if (LLVM_INCLUDE_BENCHMARKS)
  # Override benchmark defaults so that when the library itself is updated these
  # modifications are not lost.
  set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Disable benchmark testing" FORCE)
  set(BENCHMARK_ENABLE_EXCEPTIONS OFF CACHE BOOL "Disable benchmark exceptions" FORCE)
  set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "Don't install benchmark" FORCE)
  set(BENCHMARK_DOWNLOAD_DEPENDENCIES OFF CACHE BOOL "Don't download dependencies" FORCE)
  set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "Disable Google Test in benchmark" FORCE)
  set(BENCHMARK_ENABLE_WERROR ${LLVM_ENABLE_WERROR} CACHE BOOL
    "Handle -Werror for Google Benchmark based on LLVM_ENABLE_WERROR" FORCE)
  # Since LLVM requires C++11 it is safe to assume that std::regex is available.
  set(HAVE_STD_REGEX ON CACHE BOOL "OK" FORCE)
  add_subdirectory(${LLVM_THIRD_PARTY_DIR}/benchmark
    ${CMAKE_CURRENT_BINARY_DIR}/third-party/benchmark)
  add_subdirectory(benchmarks)
endif()

if (LLVM_INCLUDE_UTILS AND LLVM_INCLUDE_TOOLS)
  add_subdirectory(utils/llvm-locstats)
endif()

if (INTEL_CUSTOMIZATION)
  if (INTEL_CMAKE_DEPLOY)

    # The code below defines a set of CMake components that will
    # be put into the resulting deploy structure, which is used
    # by the product team for creating various Intel software products.
    #
    # If you have any concerns or questions about adding your new
    # component to one or more products, please contact ICLProcessTeam@intel.com
    set(INTEL_DEPLOY_COMPONENTS_LIST
      # LLVM components
      clang lld llc llvm-link llvm-profdata llvm-profgen llvm-spirv
      clang_rt.profile-x86_64 clang_rt.profile-i386 llvm-config
      clang-resource-headers llvm-cov clang-tidy llvm-ml
      # XDEV components
      xdev-dpcpp xdev-icx xdev-icx-cl xdev-icpx xdev-ifx xdev-fpp xdev-xfortcom
      xdev-fort-target-rtl xdev-compilervars xdev-il0-llvm-common-headers
      xdev-il0-llvm-common-lib xdev-support-scripts xdev-cmake-scripts
      xdev-compiler-conf
      # OpenMP components
      llvm-opencl-ompplugin omptarget clang-offload-bundler omptarget-opencl-rtl
      llvm-x86_64-ompplugin llvm-objcopy clang-offload-wrapper
      llvm-level0-ompplugin libompdevice sycl-post-link yaml2obj
# INTEL_CUSTOMIZATION
# INTEL_FEATURE_CSA
      llvm-csa-ompplugin
# end INTEL_FEATURE_CSA
# end INTEL_CUSTOMIZATION
      # SYCL components: see sycl/CMakeLists.txt
      # LLVM tools directory readme
      bin-llvm-readme
      )

    if (WIN32)
      list(APPEND INTEL_DEPLOY_COMPONENTS_LIST
        # llvm-lib is a copy of llvm-ar, and it cannot
        # be deployed without deploying llvm-ar
        llvm-ar llvm-lib
        # XDEV components
        xdev-xilib xdev-xilink
        )
    else()
      list(APPEND INTEL_DEPLOY_COMPONENTS_LIST
        # LLVM components
        llvm-ar llvm-ranlib
        # CXX components
        cxx cxxabi cxx-headers cxxabi-headers
        )

      # CMPLRLLVM-20054: icx-lto is a release name for LLVMgold plugin.
      # Darwin uses libLTO.dylib (from tools/lto), so if/when we support
      # Darwin, we will need to deploy it instead of icx-lto.
      list(APPEND INTEL_DEPLOY_COMPONENTS_LIST icx-lto)
    endif()

    if (";${LLVM_INTEL_FEATURES};" MATCHES ";INTEL_INTERNAL_BUILD;")
      list(APPEND INTEL_DEPLOY_COMPONENTS_LIST
        # Deploy sanitizers for sanitized LIT testing.
        # It should not harm to deploy them always for non-product builds.
        clang_rt.msan-x86_64 clang_rt.msan_cxx-x86_64
        clang_rt.msan-i386 clang_rt.msan_cxx-i386
        compiler-rt-headers
        # llvm-symbolizer improves traces from sanitizers.
        llvm-symbolizer
        )
    endif()

    set(INTEL_LIBDEV_DEPLOY_COMPONENTS_LIST
      # Libdev components
      libdev-libirc libdev-frtl libdev-libbfp754 libdev-libifport
      libdev-libistrconv libdev-svml libdev-libimf
    )

    list(APPEND INTEL_DEPLOY_COMPONENTS_LIST
      ${INTEL_LIBDEV_DEPLOY_COMPONENTS_LIST}
    )

    set(INTEL_OPENCL_DEPLOY_COMPONENTS_LIST
      # OpenCL components
      ocl-common_clang ocl-common_clang_emu ocl-builtins ocl-builtins-shared
      ocl-config ocl-cl_logger ocl-cl_logger_emu OpenCL-ICD
      ocl-svml ocl-gdb-plugin ocl-library-kernel
      ocl-clang_compiler ocl-clang_compiler_emu ocl-clang_compiler32_emu ocl-clang_compiler32 ocl-clang_compiler64_emu ocl-clang_compiler64
      ocl-intelocl ocl-intelocl_emu ocl-intelocl32_emu ocl-intelocl32 ocl-intelocl64_emu ocl-intelocl64
      ocl-cpu_device ocl-cpu_device_emu ocl-cpu_device32_emu ocl-cpu_device32 ocl-cpu_device64_emu ocl-cpu_device64
      ocl-task_executor ocl-task_executor_emu ocl-task_executor32_emu  ocl-task_executor32 ocl-task_executor64_emu  ocl-task_executor64
      ocl-OclCpuBackEnd ocl-OclCpuBackEnd_emu ocl-OclCpuBackEnd32 ocl-OclCpuBackEnd32_emu ocl-OclCpuBackEnd64_emu ocl-OclCpuBackEnd64
      )

    if (WIN32)
      list(APPEND INTEL_OPENCL_DEPLOY_COMPONENTS_LIST
        # OpenCL components
        ocl-OclCpuDebugging9
        )
    endif(WIN32)

    if (INTEL_OPENCL_STANDALONE_PRODUCT)
      # For standalone OpenCL product we must only deploy OpenCL components
      # and nothing else.
      set(INTEL_DEPLOY_COMPONENTS_LIST)
    endif()

    list(APPEND INTEL_DEPLOY_COMPONENTS_LIST
      ${INTEL_OPENCL_DEPLOY_COMPONENTS_LIST}
      )

    # Add SYCL deploy components defined by SYCL project.
    list(APPEND INTEL_DEPLOY_COMPONENTS_LIST ${SYCL_DEPLOY_COMPONENTS})

    # There may be duplicates due to the addition of SYCL_DEPLOY_COMPONENTS.
    list(REMOVE_DUPLICATES INTEL_DEPLOY_COMPONENTS_LIST)

    set(INTEL_DEPLOY_COMPONENTS ${INTEL_DEPLOY_COMPONENTS_LIST})
  endif()

  # This command is used for creating a fake dependency in order to force
  # another command(s) to execute.
  # Its output is never produced as a result thus each other
  # command that depends on it will always be executed.
  # We use this to force running deploy installation regardless of
  # manifest files state.
  add_custom_command(OUTPUT __force_it
                     COMMAND "${CMAKE_COMMAND}" -E echo)

  # We've seen problems with installing links, e.g. if install of llvm-ranlib
  # (link to llvm-ar) runs before llvm-ar, then CMake's create_symlink script
  # does not create the link sometime.  To resolve this (hopefully) we want to
  # run the install commands in order, so that the order of deploy components
  # passed to CMake defines the order of installation.
  set(chain_dep __force_it)

  if(DEFINED INTEL_DEPLOY_COMPONENTS)

    set(manifest_list)

    if (BUILD_SHARED_LIBS)
      # CMPLRTOOLS-14831:
      # If we are building shared libraries, we need to install
      # LLVM_LIBS.
      list(APPEND INTEL_DEPLOY_COMPONENTS ${LLVM_LIBS})
      list(APPEND INTEL_DEPLOY_COMPONENTS lldCommon lldELF lldCOFF)

      # clang libraries are in CLANG_EXPORTS list.
      # Hopefully, the extra tools, which are also in the list,
      # may be safely installed without causing any conflicts.
      get_property(CLANG_EXPORTS GLOBAL PROPERTY CLANG_EXPORTS)
      list(APPEND INTEL_DEPLOY_COMPONENTS ${CLANG_EXPORTS})
    endif()

    foreach( comp ${INTEL_DEPLOY_COMPONENTS} )

      message( STATUS "Adding component ${comp} to deploy")

      set (manifest ${CMAKE_CURRENT_BINARY_DIR}/install_manifest_${comp}.txt)
      add_custom_command(OUTPUT ${manifest}
                         COMMAND ${CMAKE_COMMAND}
                                 -DCMAKE_INSTALL_COMPONENT=${comp}
                                 -DCMAKE_INSTALL_CONFIG_NAME=$<CONFIG>
                                 -P ${CMAKE_BINARY_DIR}/cmake_install.cmake
                         # If ${comp} component does not exist, cmake
                         # will just open and immediately close the manifest
                         # file. This will not update the manifest file's ctime,
                         # if the file is located on NFSv4 share drive.
                         # We need to explicitly touch the file to force
                         # executing all deploy actions down the dependency
                         # chain.
                         COMMAND ${CMAKE_COMMAND} -E touch ${manifest}
                         DEPENDS  ${chain_dep}
                         COMMENT "Deploying component ${comp}"
                         USES_TERMINAL)
      list(APPEND manifest_list ${manifest})
      set(chain_dep ${manifest})
    endforeach( comp )

    # cmake does not allow to specify dependencies on cmakes' built-in targets such as all, clean, install etc.
    # There is feature request which filed ~9 years ago but it is still not implemented:
    # https://gitlab.kitware.com/cmake/cmake/issues/8438
    # We could work around the issue via adding custom do_build target equivalent to "make all"
    # but here another problem arises: there is no portable cmake solution to enforce parallel build.
    # Instead of specifying something like --parallel cmake option we have to explicitly pass
    # build system dependant options enforcing parallel build. Otherwise we have good chance for too slow build.
    # Thus although adding dependency on "all" is nice to have unless we have a good solution we simply allow
    # deploy target to fail if the project was not built before.
    # Leave code below just in case we change opinion.
    #
    # add_custom_target(do_build
    #                   COMMAND "${CMAKE_COMMAND}" --build . --target all )
    # add_custom_target(deploy DEPENDS do_build ${manifest_list})

    # Touch __force_it file in order to force installation regardless of manifest files state.
    add_custom_target(deploy DEPENDS ${manifest_list} COMMAND ${CMAKE_COMMAND} -E touch __force_it)
  else()
    # no specific components was given so do full installation
    set (manifest ${CMAKE_CURRENT_BINARY_DIR}/install_manifest.txt)
    add_custom_command(OUTPUT ${manifest}
                       COMMAND ${CMAKE_COMMAND}
                               -DCMAKE_INSTALL_CONFIG_NAME=$<CONFIG>
                               -P ${CMAKE_BINARY_DIR}/cmake_install.cmake
                       DEPENDS  __force_it
                       COMMENT "Performing deploy operation..."
                       USES_TERMINAL)
    # Touch __force_it file in order to force installation regardless of manifest files state.
    add_custom_target(deploy DEPENDS ${manifest} COMMAND ${CMAKE_COMMAND} -E touch __force_it)
  endif()
endif (INTEL_CUSTOMIZATION)
