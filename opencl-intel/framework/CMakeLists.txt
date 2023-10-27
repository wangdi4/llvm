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

# Embed RPATH/RUNPATH for libraries which dependencies aren't in the standard
# system libarary directories
include(embed.rpath)

use_rtti(TRUE)

include_directories(
  BEFORE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/export
  ${CL_API_HEADERS}
  ${CL_LOADER}/loader
  ${CMAKE_CURRENT_SOURCE_DIR}/Platform
  ${CMAKE_CURRENT_SOURCE_DIR}/Context
  ${CMAKE_CURRENT_SOURCE_DIR}/Execution
  ${CMAKE_CURRENT_SOURCE_DIR}/MemoryAllocator
  ${CMAKE_CURRENT_SOURCE_DIR}/Tracing
  ${OCL_BINARY_DIR})

# If llvm is built as shared libs, the shutdown process can't work properly.
if(BUILD_SHARED_LIBS)
  add_definitions(-DDISABLE_SHUTDOWN)
endif()

# For debug only
if(USE_ITT_INTERNAL)
  add_definitions(-DUSE_ITT_INTERNAL)
endif()

# these sub-directories must be added here in order to recieve the ITT
# definitions
add_subdirectory(Platform)
add_subdirectory(Context)
add_subdirectory(Execution)
add_subdirectory(MemoryAllocator)
add_subdirectory(Tracing)

# set that name of the main output file as a target name
set(TARGET_NAME intelocl${BUILD_PLATFORM})
set(TARGET_NAME_EMU ${TARGET_NAME}_${OUTPUT_EMU_SUFF})

find_sources()
calculate_target_sources()

set(FRAMEWORK_LINK_LIBS
    ${COMMON_CLANG_LIB})

if(WIN32)
  source_group("API" FILES cl_framework.cpp cl_framework.h framework_proxy.cpp
                           framework_proxy.h)
  list(APPEND FRAMEWORK_LINK_LIBS PowrProf.lib delayimp.lib lldCOFF lldCommon)
else()
  list(APPEND FRAMEWORK_LINK_LIBS cl_hw_utils)
endif(WIN32)

if(DEFINED CMAKE_IMPORT_LIBRARY_PREFIX OR DEFINED CMAKE_IMPORT_LIBRARY_SUFFIX)
  set(IMPORT_LIB
      ${CMAKE_IMPORT_LIBRARY_PREFIX}${TARGET_NAME}${CMAKE_IMPORT_LIBRARY_SUFFIX}
  )
  set(IMPORT_LIB
      ${CMAKE_IMPORT_LIBRARY_PREFIX}${TARGET_NAME_EMU}${CMAKE_IMPORT_LIBRARY_SUFFIX}
  )
endif(DEFINED CMAKE_IMPORT_LIBRARY_PREFIX OR DEFINED
                                             CMAKE_IMPORT_LIBRARY_SUFFIX)

# add a target named ${TARGET_NAME}
add_opencl_library(
  ${TARGET_NAME}
  SHARED
  ${TARGET_SOURCES}
  COMPONENTS
  Context
  Execution
  MemoryAllocator
  OclPluginManager
  Platform
  Tracing
  cl_hw_utils
  cl_logger
  cl_sys_utils
  clang_compiler
  cpu_device
  task_executor
  LINK_LIBS
  ${FRAMEWORK_LINK_LIBS}
  RC_TEMPLATE
  DEFAULT
  INSTALL_PATH
  ${OCL_INSTALL_LIBRARY_DIR})

add_opencl_library(
  ${TARGET_NAME_EMU}
  SHARED
  ${TARGET_SOURCES}
  COMPONENTS
  Context
  Execution
  MemoryAllocator
  OclPluginManager
  Platform
  Tracing
  cl_hw_utils
  cl_logger
  cl_sys_utils
  clang_compiler
  cpu_device
  task_executor
  LINK_LIBS
  ${FRAMEWORK_LINK_LIBS}
  RC_TEMPLATE
  DEFAULT
  INSTALL_PATH
  ${OCL_INSTALL_LIBRARY_DIR})

# Generate config file for FPGA emulator and cpu
if(WIN32)
  set(LINESTYLE WIN32)
else(WIN32)
  set(LINESTYLE UNIX)
endif(WIN32)

set(CL_CFG_OUTPUT_DIR ${OCL_BINARY_DIR}/lib/${OUTPUT_ARCH_SUFF})
set(CL_CFG_INSTALL_DIR ${OCL_INSTALL_LIBRARY_DIR})

set(CL_CONFIG_TBB_VERSION ${TBB_VERSION})

# For cpu
set(CL_CONFIG_DEVICES cpu)
configure_file(${OCL_SOURCE_DIR}/cmake/cl_config.in ${CL_CFG_OUTPUT_DIR}/cl.cfg
               NEWLINE_STYLE ${LINESTYLE})

# For fpga-emu
set(CL_CONFIG_DEVICES fpga-emu)
configure_file(${OCL_SOURCE_DIR}/cmake/cl_config.in
               ${CL_CFG_OUTPUT_DIR}/cl.fpga_emu.cfg NEWLINE_STYLE ${LINESTYLE})

# For fpga-emu
if(NOT (WIN32 AND "${OUTPUT_ARCH_SUFF}" STREQUAL "x86"))
  install(
    FILES ${CL_CFG_OUTPUT_DIR}/cl.fpga_emu.cfg
    DESTINATION ${CL_CFG_INSTALL_DIR}
    COMPONENT ocl-config)
endif()

# For CPU
install(
  FILES ${CL_CFG_OUTPUT_DIR}/cl.cfg
  DESTINATION ${CL_CFG_INSTALL_DIR}
  COMPONENT ocl-config)

if(WIN32)
  set_property(TARGET ${TARGET_NAME} ${TARGET_NAME_EMU} APPEND_STRING
    PROPERTY LINK_FLAGS
    " /DELAYLOAD:${COMMON_CLANG}.dll ")
endif(WIN32)

link_target_with_tbb_library(${TARGET_NAME})
link_target_with_tbb_library(${TARGET_NAME_EMU})

set_linux_exports_file(${TARGET_NAME} cl_framework_linux_exports.txt)
set_linux_exports_file(${TARGET_NAME_EMU} cl_framework_linux_exports.txt)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER ${RUNTIME_FOLDER_NAME})
set_target_properties(${TARGET_NAME_EMU} PROPERTIES FOLDER
                                                    ${RUNTIME_FOLDER_NAME})

# deploy icd loader

if(INTEL_OPENCL_STANDALONE_PRODUCT)
  install(
    TARGETS OpenCL
    DESTINATION ${OCL_INSTALL_LIBRARY_DIR}
    COMPONENT OpenCL-ICD)
endif()

install(
  TARGETS OpenCL
  DESTINATION ${OCL_OUTPUT_LIBRARY_DIR}
  COMPONENT OpenCL-ICD)
