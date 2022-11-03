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

if(NOT OPENCL_INTREE_BUILD)
  if(NOT DEFINED LLVM_PATH_FE)
    message(
      FATAL_ERROR
        "LLVM_PATH_FE is not specified. Please specify LLVM library location for backend using LLVM_PATH_FE parameter to CMAKE"
    )
  endif()
  set(LLVM_PATH ${LLVM_PATH_FE})
  find_package(LLVM REQUIRED)
endif()

# Embed RPATH/RUNPATH for libraries which dependencies aren't in the standard
# system libarary directories
include(embed.rpath)

# this component has to use deprecated OpenCL API functions.
if(WIN32)
  add_definitions(-wd4996 # Suppress 'function': was declared deprecated'
  )
endif(WIN32)

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

if(WIN32)
  source_group("API" FILES cl_framework.cpp cl_framework.h framework_proxy.cpp
                           framework_proxy.h)

  list(APPEND LINK_LIBS PowrProf.lib)
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

link_directories(${TBB_LIB_DIR})

include(itt)

set(FRAMEWORK_LINK_LIBS
    Context
    Execution
    Platform
    MemoryAllocator
    Tracing
    cl_sys_utils
    CLElfLib
    LLVMBitReader
    LLVMSupport)

# add a target named ${TARGET_NAME}
add_opencl_library(
  ${TARGET_NAME}
  SHARED
  ${TARGET_SOURCES}
  COMPONENTS
  task_executor${BUILD_PLATFORM}
  cl_logger
  LINK_LIBS
  ${FRAMEWORK_LINK_LIBS}
  ${LINK_LIBS}
  RC_TEMPLATE
  DEFAULT
  INSTALL_PATH
  ${OUTPUT_ARCH_SUFF})

add_opencl_library(
  ${TARGET_NAME_EMU}
  SHARED
  ${TARGET_SOURCES}
  COMPONENTS
  task_executor${BUILD_PLATFORM}_${OUTPUT_EMU_SUFF}
  cl_logger_${OUTPUT_EMU_SUFF}
  LINK_LIBS
  ${FRAMEWORK_LINK_LIBS}
  ${LINK_LIBS}
  RC_TEMPLATE
  DEFAULT
  INSTALL_PATH
  ${OUTPUT_ARCH_SUFF})

# Generate config file for FPGA emulator and cpu
if(WIN32)
  set(LINESTYLE WIN32)
else(WIN32)
  set(LINESTYLE UNIX)
endif(WIN32)

set(CL_CFG_OUTPUT_DIR ${OCL_BINARY_DIR}/lib/${OUTPUT_ARCH_SUFF})
set(CL_CFG_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib/${OUTPUT_ARCH_SUFF})

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
  set_target_properties(
    ${TARGET_NAME} PROPERTIES LINK_FLAGS
                              "/DELAYLOAD:task_executor${BUILD_PLATFORM}.dll")
  get_target_property(LK_FLAGS_DEBUG ${TARGET_NAME} LINK_FLAGS_DEBUG)
  set_target_properties(
    ${TARGET_NAME} PROPERTIES LINK_FLAGS_DEBUG
                              "${LK_FLAGS_DEBUG} /NODEFAULTLIB:MSVCRT")
  set_target_properties(
    ${TARGET_NAME_EMU}
    PROPERTIES
      LINK_FLAGS
      "/DELAYLOAD:task_executor${BUILD_PLATFORM}_${OUTPUT_EMU_SUFF}.dll")
  get_target_property(LK_FLAGS_DEBUG ${TARGET_NAME_EMU} LINK_FLAGS_DEBUG)
  set_target_properties(
    ${TARGET_NAME_EMU} PROPERTIES LINK_FLAGS_DEBUG
                                  "${LK_FLAGS_DEBUG} /NODEFAULTLIB:MSVCRT")

  list(APPEND LINK_LIBS delayimp.lib)
endif(WIN32)

set_linux_exports_file(${TARGET_NAME} cl_framework_linux_exports.txt)
set_linux_exports_file(${TARGET_NAME_EMU} cl_framework_linux_exports.txt)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER ${RUNTIME_FOLDER_NAME})
set_target_properties(${TARGET_NAME_EMU} PROPERTIES FOLDER
                                                    ${RUNTIME_FOLDER_NAME})

# deploy icd loader

if(INTEL_OPENCL_STANDALONE_PRODUCT)
  install(
    TARGETS OpenCL
    DESTINATION lib/${OUTPUT_ARCH_SUFF}
    COMPONENT OpenCL-ICD)
endif()

install(
  TARGETS OpenCL
  DESTINATION ${OCL_OUTPUT_LIBRARY_DIR}
  COMPONENT OpenCL-ICD)
