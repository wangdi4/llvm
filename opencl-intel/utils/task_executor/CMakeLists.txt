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
# Project variables: OCL_SOURCE_DIR    - points to the top src directory
# OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root
#                     of generated files
#

# set that name of the main output file as a target name
set(TARGET_NAME task_executor${BUILD_PLATFORM})

# set emulator target name
# FIXME: that is a hack to build both CPU and FPGA emulator libs within a single
# building process. Better solutions:
# - Merge _all_ CPU RT libraries into one;
# - Make both CPU and FPGA emu platforms be accessible from a single binary set.
set(TARGET_NAME_EMU ${TARGET_NAME}_${OUTPUT_EMU_SUFF})

# Embed RPATH/RUNPATH for libraries which dependencies aren't in the standard
# system library directories
include(embed.rpath)
include(itt)

if(WIN32 AND NOT BUILD_X64)
  # the /Ob1 flag is set instead of /Ob2 flag in order to prevent aggressive
  # inlining optimizations in Visual Studio due to a bug discovered in device
  # fission: CSSD100013570
  string(REPLACE /Ob2 "/Ob1" CMAKE_C_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
  string(REPLACE /Ob2 "/Ob1" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
endif(WIN32 AND NOT BUILD_X64)
if(NOT WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -faligned-new")
endif(NOT WIN32)

link_directories(${TBB_LIB_DIR})

add_definitions(-DTASK_EXECUTOR_EXPORTS -D__TBB_EXECUTOR__ -D__TBB_EXTRA_DEBUG
                -D__TBB_NO_IMPLICIT_LINKAGE=1 -D_USRDLL)

set_unicode_off()

# only tbb executor is supported on Linux
if(NOT WIN32)
  set(IGNORE_FILES thread_executor|xn_)
endif(NOT WIN32)

find_sources(${IGNORE_FILES})
calculate_target_sources()

# add a target named ${TARGET_NAME}
add_opencl_library(
  ${TARGET_NAME}
  SHARED
  ${TARGET_SOURCES}
  INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR} # keep it here for windows
  ${CL_API_HEADERS}
  ${OCL_BINARY_DIR}
  RC_TEMPLATE
  DEFAULT
  INSTALL_PATH
  ${OUTPUT_ARCH_SUFF})

# add an FPGA emulator library
add_opencl_library(
  ${TARGET_NAME_EMU}
  SHARED
  ${TARGET_SOURCES}
  INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR} # keep it here for windows
  ${CL_API_HEADERS}
  ${OCL_BINARY_DIR}
  RC_TEMPLATE
  DEFAULT
  INSTALL_PATH
  ${OUTPUT_ARCH_SUFF})

if(WIN32)
  list(APPEND LINK_LIBS delayimp.lib legacy_stdio_definitions.lib)
else(WIN32)
  if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    list(APPEND LINK_LIBS ${IMPLIB_PREFIX}tbb_debug${IMPLIB_SUFFIX})
    list(APPEND LINK_LIBS ${IMPLIB_PREFIX}tbbmalloc_debug${IMPLIB_SUFFIX})
  else()
    list(APPEND LINK_LIBS ${IMPLIB_PREFIX}tbb${IMPLIB_SUFFIX})
    list(APPEND LINK_LIBS ${IMPLIB_PREFIX}tbbmalloc${IMPLIB_SUFFIX})
  endif(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
endif(WIN32)

target_link_libraries(
  ${TARGET_NAME} ${LINK_LIBS} cl_logger${OPENCL_BINARIES_POSTFIX} cl_sys_utils
  cl_hw_utils)
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER ${UTILS_FOLDER_NAME})

target_link_libraries(${TARGET_NAME_EMU} ${LINK_LIBS}
                      cl_logger_${OUTPUT_EMU_SUFF} cl_sys_utils cl_hw_utils)
set_target_properties(${TARGET_NAME_EMU} PROPERTIES FOLDER ${UTILS_FOLDER_NAME})

if(WIN32)
  get_target_property(LK_FLAGS_DEBUG ${TARGET_NAME} LINK_FLAGS_DEBUG)
  get_target_property(LK_FLAGS_RELEASE ${TARGET_NAME} LINK_FLAGS_RELEASE)
  set_target_properties(
    ${TARGET_NAME}
    PROPERTIES
      LINK_FLAGS_DEBUG
      "${LK_FLAGS_DEBUG} tbbmalloc_debug.lib tbb12_debug${TBB_BINARIES_POSTFIX}.lib /DELAYLOAD:tbbmalloc_debug.dll /DELAYLOAD:tbb12_debug${TBB_BINARIES_POSTFIX}.dll"
      LINK_FLAGS_RELEASE
      "${LK_FLAGS_RELEASE} tbbmalloc.lib tbb12${TBB_BINARIES_POSTFIX}.lib /DELAYLOAD:tbbmalloc.dll  /DELAYLOAD:tbb12${TBB_BINARIES_POSTFIX}.dll"
  )

  get_target_property(LK_FLAGS_DEBUG ${TARGET_NAME_EMU} LINK_FLAGS_DEBUG)
  get_target_property(LK_FLAGS_RELEASE ${TARGET_NAME_EMU} LINK_FLAGS_RELEASE)
  set_target_properties(
    ${TARGET_NAME_EMU}
    PROPERTIES
      LINK_FLAGS_DEBUG
      "${LK_FLAGS_DEBUG} tbbmalloc_debug.lib tbb12_debug${TBB_BINARIES_POSTFIX}.lib /DELAYLOAD:tbbmalloc_debug.dll /DELAYLOAD:tbb12_debug${TBB_BINARIES_POSTFIX}.dll"
      LINK_FLAGS_RELEASE
      "${LK_FLAGS_RELEASE} tbbmalloc.lib tbb12${TBB_BINARIES_POSTFIX}.lib /DELAYLOAD:tbbmalloc.dll  /DELAYLOAD:tbb12${TBB_BINARIES_POSTFIX}.dll"
  )
else()
  set_linux_exports_file(${TARGET_NAME} task_executor_linux_exports.txt)
  set_linux_exports_file(${TARGET_NAME_EMU} task_executor_linux_exports.txt)
endif(WIN32)
