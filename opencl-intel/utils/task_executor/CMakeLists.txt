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
set(TARGET_NAME task_executor)

# Embed RPATH/RUNPATH for libraries which dependencies aren't in the standard
# system library directories
include(embed.rpath)

if(WIN32 AND NOT BUILD_X64)
  # the /Ob1 flag is set instead of /Ob2 flag in order to prevent aggressive
  # inlining optimizations in Visual Studio due to a bug discovered in device
  # fission
  string(REPLACE /Ob2 "/Ob1" CMAKE_C_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
  string(REPLACE /Ob2 "/Ob1" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
endif(WIN32 AND NOT BUILD_X64)
if(NOT WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -faligned-new")
endif(NOT WIN32)

add_definitions(-DTASK_EXECUTOR_EXPORTS -D__TBB_EXECUTOR__ -D__TBB_EXTRA_DEBUG
                -D__TBB_NO_IMPLICIT_LINKAGE=1 -D_USRDLL
                -DDEFAULT_OCL_LIBRARY_DIR="${OCL_OUTPUT_LIBRARY_DIR}")

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
  STATIC
  ${TARGET_SOURCES}
  INCLUDE_DIRS
  ${CL_API_HEADERS}
  ${CMAKE_CURRENT_SOURCE_DIR} # keep it here for windows
  ${OCL_BINARY_DIR}
  LINK_LIBS
  LLVMIntelJITEvents
  cl_logger
  cl_sys_utils)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER ${UTILS_FOLDER_NAME})

if(WIN32)
  # Copy TBB libraries to deploy and build folder. This is to support windows
  # QA. TBB is used by task_executor.
  file(GLOB TBB_FILES_TO_DEPLOY
       "${TBB_LIBRARY_DIR}/*.dll"
       "${TBB_LIBRARY_DIR}/*.pdb")
  set(TBB_BINARIES_BUILD_DIR
      ${OCL_BINARY_DIR}/lib/${OUTPUT_ARCH_SUFF})
  add_custom_target(
    copy_tbb
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${TBB_FILES_TO_DEPLOY}
            ${TBB_BINARIES_BUILD_DIR}
    COMMENT "Copying tbb libraries from ${TBB_LIBRARY_DIR} to ${TBB_BINARIES_BUILD_DIR}")
  add_dependencies(${TARGET_NAME} copy_tbb)

  install_to(${TBB_FILES_TO_DEPLOY} COMPONENT ocl-tbb DESTINATION
             ${OCL_INSTALL_LIBRARY_DIR})
endif()
