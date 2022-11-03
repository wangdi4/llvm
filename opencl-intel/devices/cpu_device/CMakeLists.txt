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
set(TARGET_NAME cpu_device${BUILD_PLATFORM})

# set emulator target name
# FIXME: that is a hack to build both CPU and FPGA emulator libs within a single
# building process. Better solutions:
# - Merge _all_ CPU RT libraries into one;
# - Make both CPU and FPGA emu platforms be accessible from a single binary set.
set(TARGET_NAME_EMU ${TARGET_NAME}_${OUTPUT_EMU_SUFF})

set(CMAKE_CXX_FLAGS_DEBUG "-DOCLDEVICE_PLUGINS ${CMAKE_CXX_FLAGS_DEBUG}")

if(NOT WIN32)
  set(HW_UTILS_HEADERS ${OCL_SOURCE_DIR}/utils/cl_hw_utils/export)
  set(HW_UTILS_PROJ_NAME cl_hw_utils)
endif(NOT WIN32)

include_directories(
  BEFORE
  ${OCL_SOURCE_DIR}/backend/ocl_cpu_backend/export
  ${OCL_SOURCE_DIR}/FrontendDriver
  ${OCL_SOURCE_DIR}/cl_api
  ${HW_UTILS_HEADERS}
  ${OCL_BINARY_DIR}
  ${CL_API_HEADERS}
  ../common/builtin_kernels)

link_directories(${TBB_LIB_DIR})

if(WIN32)
  add_definitions(-D_WINDOWS)
else()
  add_definitions(-D_LINUX)
endif(WIN32)

add_definitions(-D__TBB_NO_IMPLICIT_LINKAGE=1)

# Support for built-in kernels
list(APPEND LINK_LIBS builtin_kernels device_commands)

set(MKL_LINK_LIBS)
# Add MKL library integration
if(INCLUDE_MKL)
  list(APPEND MKL_LINK_LIBS mkl_kernels)
  include_directories(BEFORE ../common/mkl_kernels/export)
  if(WIN32)
    link_directories(BEFORE ${MKL_DIR}/lib/${ICC_ARCH_DIR}/)
    list(APPEND MKL_LINK_LIBS mkl_intel_ilp64 mkl_intel_thread mkl_core)
  else()
    # Next to lines uses dynamic libraries from MKL. This to be resolved against
    # linkage with static libraries link_directories( BEFORE
    # ${MKL_DIR}/lib/${ICC_ARCH_DIR}/ ) list(APPEND LINK_LIBS mkl_intel_ilp64
    # mkl_intel_thread mkl_core)
    list(
      APPEND
      MKL_LINK_LIBS
      "-Wl,--start-group ${MKL_DIR}/lib/${ICC_ARCH_DIR}/libmkl_intel_ilp64.a ${MKL_DIR}/lib/${ICC_ARCH_DIR}/libmkl_core.a ${MKL_DIR}/lib/${ICC_ARCH_DIR}/libmkl_intel_thread.a -Wl,--end-group"
    )
    list(APPEND MKL_LINK_LIBS dl pthread m)
  endif(WIN32)

  if(USE_OMP2TBB)
    list(APPEND MKL_LINK_LIBS omp2tbb)
  else(USE_OMP2TBB)
    if(WIN32)
      list(APPEND MKL_LINK_LIBS
           ${ICC_DIR}/compiler/lib/${ICC_ARCH_DIR}/libiomp5md.lib)
    else()
      list(APPEND MKL_LINK_LIBS
           ${ICC_DIR}/compiler/lib/${ICC_ARCH_DIR}/libiomp5.so)
    endif(WIN32)
  endif(USE_OMP2TBB)
endif(INCLUDE_MKL)

find_sources()
calculate_target_sources()

# include ITT notify sources
include(itt)

if(WIN32)
  list(APPEND LINK_LIBS delayimp.lib)
endif(WIN32)

# add a target named ${TARGET_NAME}
add_opencl_library(
  ${TARGET_NAME}
  SHARED
  ${TARGET_SOURCES}
  COMPONENTS
  task_executor${BUILD_PLATFORM}
  cl_logger
  LINK_LIBS
  ${HW_UTILS_PROJ_NAME}
  cl_sys_utils
  device_commands
  ${LINK_LIBS}
  RC_TEMPLATE
  DEFAULT
  INSTALL_PATH
  ${OUTPUT_ARCH_SUFF})

# add an FPGA emulator library
add_opencl_library(
  ${TARGET_NAME_EMU}
  SHARED
  ${TARGET_SOURCES}
  COMPONENTS
  task_executor${BUILD_PLATFORM}_${OUTPUT_EMU_SUFF}
  cl_logger_${OUTPUT_EMU_SUFF}
  LINK_LIBS
  ${HW_UTILS_PROJ_NAME}
  cl_sys_utils
  device_commands
  ${LINK_LIBS}
  RC_TEMPLATE
  DEFAULT
  INSTALL_PATH
  ${OUTPUT_ARCH_SUFF})

set_linux_exports_file(${TARGET_NAME} cpu_device_linux_exports.txt)
set_linux_exports_file(${TARGET_NAME_EMU} cpu_device_linux_exports.txt)

if(INCLUDE_MKL)
  target_link_libraries(${TARGET_NAME} LINK_PRIVATE ${MKL_LINK_LIBS})
  # FIXME: do we have support of MKL on the emulator?
  target_link_libraries(${TARGET_NAME_EMU} LINK_PRIVATE ${MKL_LINK_LIBS})
endif(INCLUDE_MKL)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER ${DEVICES_FOLDER_NAME})
set_target_properties(${TARGET_NAME_EMU} PROPERTIES FOLDER
                                                    ${DEVICES_FOLDER_NAME})

if(WIN32)
  get_target_property(LK_FLAGS_DEBUG ${TARGET_NAME} LINK_FLAGS_DEBUG)
  get_target_property(LK_FLAGS_RELEASE ${TARGET_NAME} LINK_FLAGS_RELEASE)
  set_target_properties(
    ${TARGET_NAME}
    PROPERTIES
      LINK_FLAGS_DEBUG
      "${LK_FLAGS_DEBUG} /NODEFAULTLIB:MSVCRT /DELAYLOAD:tbbmalloc_debug.dll /DELAYLOAD:task_executor${BUILD_PLATFORM}.dll"
      LINK_FLAGS_RELEASE
      "${LK_FLAGS_RELEASE} /DELAYLOAD:tbbmalloc.dll /DELAYLOAD:task_executor${BUILD_PLATFORM}.dll"
  )

  get_target_property(LK_FLAGS_DEBUG ${TARGET_NAME_EMU} LINK_FLAGS_DEBUG)
  get_target_property(LK_FLAGS_RELEASE ${TARGET_NAME_EMU} LINK_FLAGS_RELEASE)
  set_target_properties(
    ${TARGET_NAME_EMU}
    PROPERTIES
      LINK_FLAGS_DEBUG
      "${LK_FLAGS_DEBUG} /NODEFAULTLIB:MSVCRT /DELAYLOAD:tbbmalloc_debug.dll /DELAYLOAD:task_executor${BUILD_PLATFORM}_${OUTPUT_EMU_SUFF}.dll"
      LINK_FLAGS_RELEASE
      "${LK_FLAGS_RELEASE} /DELAYLOAD:tbbmalloc.dll /DELAYLOAD:task_executor${BUILD_PLATFORM}_${OUTPUT_EMU_SUFF}.dll"
  )
else(WIN32)
  if(INCLUDE_MKL)
    # need explicitly set the full path to prevent failure in cpu_device_type
    # compilation
    set_property(
      TARGET ${TARGET_NAME}
      APPEND_STRING
      PROPERTY LINK_FLAGS " -Wl,-z,defs")
    set_property(
      TARGET ${TARGET_NAME_EMU}
      APPEND_STRING
      PROPERTY LINK_FLAGS " -Wl,-z,defs")
  endif(INCLUDE_MKL)
endif(WIN32)
