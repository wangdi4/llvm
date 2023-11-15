#
# Copyright (C) 2023 Intel Corporation
#
# This software and the related documents are Intel copyrighted materials, and
# your use of them is governed by the express license under which they were
# provided to you ("License"). Unless the License provides otherwise, you may
# not use, modify, copy, publish, distribute, disclose or transmit this
# software or the related documents without Intel's prior written permission.
#
# This software and the related documents are provided as is, with no express
# or implied warranties, other than those that are expressly stated in the
# License.

if(INTEL_CUSTOMIZATION)
  if(DEFINED ENV{ICS_GIT_MIRROR} AND NOT "$ENV{ICS_GIT_MIRROR}" STREQUAL "")
    STRING(REGEX REPLACE "\\\\" "/" GITSERVER "$ENV{ICS_GIT_MIRROR}")
  else()
    set(GITSERVER "https://github.com/intel-innersource")
  endif()
 set(OPENCL_CLANG_REPO
   "${GITSERVER}/applications.compilers.source.ocl-common-clang")
endif(INTEL_CUSTOMIZATION)

# Download opencl-clang
set(OPENCL_CLANG_TAG ad248f4104c1fa37684a87c6d8ff74cc4fac61d4)
# Set target name
set(OPENCL_CLANG_LIBRARY_NAME "common_clang" CACHE STRING "")
# Override PCH extension map in opencl-clang/options_compile.cpp
set(OPENCL_CLANG_PCH_EXTENSION "cl_khr_3d_image_writes,cl_khr_depth_images,cl_khr_global_int32_base_atomics,cl_khr_global_int32_extended_atomics,cl_khr_int64_base_atomics,cl_khr_int64_extended_atomics,cl_khr_local_int32_base_atomics,cl_khr_local_int32_extended_atomics,cl_intel_subgroups,cl_intel_subgroups_short" CACHE STRING "")
# Exclude unneeded libraries
llvm_map_components_to_libnames(ALL_LLVM_LIBS all)
list(REMOVE_ITEM ALL_LLVM_LIBS LLVMBitReader LLVMBitWriter LLVMCore LLVMOption LLVMSupport)
if(WIN32)
  # Don't link LLVMSupport_dyn which will propogate /MD flag to opencl-clang.
  list(APPEND ALL_LLVM_LIBS LLVMSupport_dyn)
endif()
set(OPENCL_CLANG_EXCLUDE_LIBS_FROM_ALL "${ALL_LLVM_LIBS}" CACHE STRING "")

message(STATUS "Will fetch opencl-clang from ${OPENCL_CLANG_REPO}")
FetchContent_Declare(
  opencl-clang
  GIT_REPOSITORY ${OPENCL_CLANG_REPO}
  GIT_TAG ${OPENCL_CLANG_TAG}
)

FetchContent_MakeAvailable(opencl-clang)

if (NOT EXISTS ${opencl-clang_SOURCE_DIR})
  message(FATAL_ERROR "opencl-clang is not fetched from ${OPENCL_CLANG_REPO}")
endif()

macro(set_opencl_clang_extra_properties)
  set(COMMON_CLANG common_clang${BUILD_PLATFORM})
  if(OPENCL_INTREE_BUILD)
    set(CLANG_OPENCL_HEADERS_DIR ${LLVM_EXTERNAL_CLANG_SOURCE_DIR}/lib/Headers)

    get_target_property(COMMON_CLANG_H_LOC ${COMMON_CLANG} SOURCE_DIR)
    if((NOT COMMON_CLANG_H_LOC) OR (NOT EXISTS
                                    "${COMMON_CLANG_H_LOC}/opencl_clang.h"))
      message(FATAL_ERROR "Cannot find opencl_clang.h from opencl_clang project (in ${COMMON_CLANG_H_LOC}).")
    endif()
    set(CCLANG_DEV_INCLUDE_DIRS ${COMMON_CLANG_H_LOC})
  endif()

  # Prepare common_clang dynamic library
  if(UNIX)
    set_target_properties(${COMMON_CLANG}
      PROPERTIES
      SOVERSION ${VERSIONSTRING}
      VERSION ${VERSIONSTRING})
    if (LLVM_LINKER_IS_LLD)
      # Newer LLD changed behavior to consider references to undefined symbols
      # to be an error. This option ensures we use the older behavior.
      target_link_options(${COMMON_CLANG} PRIVATE "LINKER:--undefined-version")
    endif (LLVM_LINKER_IS_LLD)
  endif()
  if(OPENCL_INTREE_BUILD)
    set(COMMON_CLANG_LIB ${COMMON_CLANG})
  else()
    if(WIN32)
      set(COMMON_CLANG_LIB ${LLVM_LIBRARY_DIRS}/${COMMON_CLANG}.lib)
    else()
      set(COMMON_CLANG_LIB
          ${LLVM_LIBRARY_DIR}/lib${COMMON_CLANG}.so.${VERSIONSTRING})
      set(COMMON_CLANG_SYMLINK ${LLVM_LIBRARY_DIR}/lib${COMMON_CLANG}.so)
    endif()
  endif()

  # Deploy
  if(WIN32)
    if(OPENCL_INTREE_BUILD)
      set(COMMON_CLANG_BIN_DIR ${LLVM_BINARY_DIR}/bin)
    else()
      set(COMMON_CLANG_BIN_DIR ${LLVM_BINARY_DIR})
    endif()
    set(COMMON_CLANG_FILES ${COMMON_CLANG_BIN_DIR}/${COMMON_CLANG}.dll)
  else(WIN32)
    set(COMMON_CLANG_FILES ${COMMON_CLANG_LIB} ${COMMON_CLANG_SYMLINK})
  endif(WIN32)
  if(WIN32)
    get_ics_build_type(ICS_BUILD_TYPE)
    # Disable PDB installation in release build
    if(NOT ICS_BUILD_TYPE STREQUAL "release")
      list(APPEND COMMON_CLANG_FILES ${COMMON_CLANG_BIN_DIR}/${COMMON_CLANG}.pdb)
    endif()
  endif(WIN32)

  if(NOT OPENCL_INTREE_BUILD)
    copy_to(${COMMON_CLANG_FILES} DESTINATION lib/${OUTPUT_ARCH_SUFF})
    copy_to(${COMMON_CLANG_FILES} DESTINATION lib/${OUTPUT_EMU_SUFF})
  endif()

  install_to(${COMMON_CLANG_FILES} DESTINATION ${OCL_INSTALL_LIBRARY_DIR}
             COMPONENT ocl-common_clang)
endmacro()
