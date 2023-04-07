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

# binary output directory suffix
if(BUILD_X64)
  set(BIN_OUTPUT_DIR_SUFFIX win64)
else(BUILD_X64)
  set(BIN_OUTPUT_DIR_SUFFIX win32)
endif(BUILD_X64)

# Microsoft Assembler setup - use private rules
if(BUILD_X64)
  set(OPENCL_ASM_COMPILER ml64) # OLD changed due to issues in TFW (didn't find
                                # asm compiler on win 64
  set(CMAKE_ASM_FLAGS /nologo /c /W3 /errorReport:prompt) # do not quote
                                                          # this!!!!
else(BUILD_X64)
  set(OPENCL_ASM_COMPILER ml)
  set(CMAKE_ASM_FLAGS /nologo /safeseh /c /coff /W3 /errorReport:prompt
  )# do not quote this!!!!
endif(BUILD_X64)

set(CMAKE_ASM_INCLUDE_DIR_FLAG /I)
set(CMAKE_ASM_OUTPUT_NAME_FLAG /Fo)
set(CMAKE_ASM_LINK_FLAG /nologo)

# Compiler switches that CANNOT be modified during makefile generation
# We add _HAS_STD_BYTE=0 definition according to HandleLLVMOptions.cmake
set(ADD_C_FLAGS "/Oi -D WINDOWS_ENABLE_CPLUSPLUS /GS -D _HAS_STD_BYTE=0")
set(ADD_C_FLAGS_DEBUG "-D _DEBUG /RTC1")
set(ADD_C_FLAGS_RELEASE "/Gy")

if(NOT DEFINED INTEL_COMPILER)
  set(ADD_C_FLAGS_RELEASE "${ADD_C_FLAGS_RELEASE} /GS")
endif()

# Compiler switches that CAN be modified during makefile generation and
# configuration-independent
add_definitions(-DWIN32)

# INTEL_CUSTOMIZATION
# ITT/GPA/VTUNE integration
if(USE_GPA)
  add_definitions(-DUSE_GPA)
  include_directories(${CMAKE_SOURCE_DIR}/externals/Intel-gpa/include)
endif(USE_GPA)
# end INTEL_CUSTOMIZATION
# Linker switches
if(BUILD_X64)
  set(INIT_LINKER_FLAGS
      "/MACHINE:X64 /INCREMENTAL:NO /DYNAMICBASE /NXCOMPAT")
else(BUILD_X64)
  set(INIT_LINKER_FLAGS
      "/MACHINE:X86 /INCREMENTAL:NO /DYNAMICBASE /NXCOMPAT /SAFESEH")
endif(BUILD_X64)

set(ADD_LINKER_FLAGS_DEBUG "/DEBUG /NODEFAULTLIB:LIBCMT /NODEFAULTLIB:LIBCPMT")
set(ADD_LINKER_FLAGS_RELEASE
    "/NODEFAULTLIB:LIBCMTD /NODEFAULTLIB:LIBCPMTD")

# setup
set(CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})

# remove /INCREMENTAL:YES option from DEBUG Linker switches
string(REPLACE /INCREMENTAL:YES "" CMAKE_EXE_LINKER_FLAGS_DEBUG
               ${CMAKE_EXE_LINKER_FLAGS_DEBUG})
string(REPLACE /INCREMENTAL:YES "" CMAKE_SHARED_LINKER_FLAGS_DEBUG
               ${CMAKE_SHARED_LINKER_FLAGS_DEBUG})

# C switches
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}         ${ADD_C_FLAGS}")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}   ${ADD_C_FLAGS_DEBUG}")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${ADD_C_FLAGS_RELEASE}")

# C++ switches
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}         ${ADD_C_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}   ${ADD_C_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${ADD_C_FLAGS_RELEASE}")

# Linker switches - EXE
set(CMAKE_EXE_LINKER_FLAGS ${INIT_LINKER_FLAGS})
set(CMAKE_EXE_LINKER_FLAGS_DEBUG
    "${CMAKE_EXE_LINKER_FLAGS_DEBUG}   ${ADD_LINKER_FLAGS_DEBUG}")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE
    "${CMAKE_EXE_LINKER_FLAGS_RELEASE} ${ADD_LINKER_FLAGS_RELEASE}")

# Linker switches - DLL
set(CMAKE_SHARED_LINKER_FLAGS ${INIT_LINKER_FLAGS})
set(CMAKE_SHARED_LINKER_FLAGS_DEBUG
    "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}   ${ADD_LINKER_FLAGS_DEBUG}")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE
    "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} ${ADD_LINKER_FLAGS_RELEASE}")

if(UWD_BUILD)
  set(CMAKE_SYSTEM_VERSION 10.0)
  ocl_replace_compiler_option(CMAKE_C_FLAGS_RELEASE "/MD" "/MT")
  ocl_replace_compiler_option(CMAKE_CXX_FLAGS_RELEASE "/MD" "/MT")
  ocl_replace_compiler_option(CMAKE_C_FLAGS_DEBUG "/MDd" "/MTd")
  ocl_replace_compiler_option(CMAKE_CXX_FLAGS_DEBUG "/MDd" "/MTd")
endif()

# Intel new unified layout requires that all 64-bit dynamic libraries to be
# installed into 'bin' dir and all 32-bit dynamic libraries to be installed
# into 'bin32' dir.
if(INTEL_DEPLOY_UNIFIED_LAYOUT)
  if(BUILD_X64)
    set(OCL_INSTALL_LIBRARY_DIR bin)
  else()
    set(OCL_INSTALL_LIBRARY_DIR bin32)
  endif(BUILD_X64)
else()
  set(OCL_INSTALL_LIBRARY_DIR lib/${OUTPUT_ARCH_SUFF})
endif(INTEL_DEPLOY_UNIFIED_LAYOUT)
