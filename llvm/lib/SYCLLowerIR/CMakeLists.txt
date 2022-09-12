# INTEL_CUSTOMIZATION
#
# INTEL CONFIDENTIAL
#
# Modifications, Copyright (C) 2021 Intel Corporation
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
if(${CMAKE_VERSION} VERSION_LESS 3.14)
    macro(FetchContent_MakeAvailable NAME)
        FetchContent_GetProperties(${NAME})
        if(NOT ${NAME}_POPULATED)
            FetchContent_Populate(${NAME})
            add_subdirectory(${${NAME}_SOURCE_DIR} ${${NAME}_BINARY_DIR})
        endif()
    endmacro()
endif()

# Lowering of SYCL ESIMD kernels depends on vc-intrinsics
# NOTE: could have been added earlier from llvm/projects
if (NOT TARGET LLVMGenXIntrinsics)
  if (NOT DEFINED LLVMGenXIntrinsics_SOURCE_DIR)
    set(LLVMGenXIntrinsics_GIT_REPO https://github.com/intel/vc-intrinsics.git)
    # Author: Haohai Wen <haohai.wen@intel.com>
    # Date:   Fri Jun 10 03:46:22 2022 +0000
    # Format InstructionSimplify names
    set(LLVMGenXIntrinsics_GIT_TAG abce9184b7a3a7fe1b02289b9285610d9dc45465)

    if (INTEL_CUSTOMIZATION)
      # for xmain builds the LLVMGenXIntrinsics_GIT_REPO is constructed from
      # GIT_REPO_PATH and GIT_REPO_TAG set independently depending on build
      # configuration (public/embargo) and condition (building from under ICS
      # or not)

      # default repo path for public xmain builds, not using ICS
      set(LLVMGenXIntrinsics_GIT_REPO_PATH https://github.com/intel-innersource)
      set(LLVMGenXIntrinsics_GIT_REPO_NAME applications.compilers.source.vc-intrinsics.git)
      # Author: Haohai Wen <haohai.wen@intel.com>
      # Date:   Fri Jun 10 03:46:22 2022 +0000
      # Format InstructionSimplify names
      set(LLVMGenXIntrinsics_GIT_TAG abce9184b7a3a7fe1b02289b9285610d9dc45465)

      # Determine if the build is embargo and provide specific settings.
      set(IS_INTERNAL_ESIMD_BUILD FALSE)
# INTEL_FEATURE_ESIMD_EMBARGO
      is_intel_feature_enabled(IS_INTERNAL_ESIMD_BUILD ${LLVM_INTELFEATURE_PREFIX}_ESIMD_EMBARGO)

      if (IS_INTERNAL_ESIMD_BUILD)
        # override git repo name and tag for the internal ESIMD EMBARGO build
        set(LLVMGenXIntrinsics_GIT_REPO_NAME drivers.gpu.compute.vc-intrinsics.git)
        # Author: HaohaiWen <haohai.wen@intel.com>
        # Date:   Fri Jun 10 23:43:35 2022 +0800
        # [VC BugFix]: Format InstructionSimplify names (#202)
        set(LLVMGenXIntrinsics_GIT_TAG 6bef8e76ec6a39c324c8fa350580f9a55e661656)

        add_compile_definitions(__INTEL_EMBARGO__)
        # LLVM_CMAKE_DIR is required by the internal vc-intrinsics repo, but is
        # not set for non-standalone builds. Use LLVM_CMAKE_PATH instead.
        if (NOT LLVM_CMAKE_DIR)
          set(LLVM_CMAKE_DIR ${LLVM_CMAKE_PATH})
        endif (NOT LLVM_CMAKE_DIR)
      endif (IS_INTERNAL_ESIMD_BUILD)
# end INTEL_FEATURE_ESIMD_EMBARGO

      if (DEFINED ENV{ICS_GIT_MIRROR} AND NOT "$ENV{ICS_GIT_MIRROR}" STREQUAL "")
        # ICS_GIT_MIRROR, if set, overrides GIT_REPO_PATH.
        set(LLVMGenXIntrinsics_GIT_REPO_PATH $ENV{ICS_GIT_MIRROR})
        STRING(REGEX REPLACE "\\\\" "/" LLVMGenXIntrinsics_GIT_REPO_PATH "${LLVMGenXIntrinsics_GIT_REPO_PATH}")
# INTEL_FEATURE_ESIMD_EMBARGO
      elseif (IS_INTERNAL_ESIMD_BUILD)
        # ICS_GIT_MIRROR is obligatory for embargo builds.
        message( FATAL_ERROR "xmain ESIMD_EMBARGO build requires ICS_GIT_MIRROR env var to be set" )
# end INTEL_FEATURE_ESIMD_EMBARGO
      endif ()

      # Construct GIT_REPO from from GIT_REPO_PATH and GIT_REPO_NAME.
      set( LLVMGenXIntrinsics_GIT_REPO
        ${LLVMGenXIntrinsics_GIT_REPO_PATH}/${LLVMGenXIntrinsics_GIT_REPO_NAME} )
    endif (INTEL_CUSTOMIZATION)

    message(STATUS "vc-intrinsics repo is missing. Will try to download it from ${LLVMGenXIntrinsics_GIT_REPO}")
    include(FetchContent)
    FetchContent_Declare(vc-intrinsics
      GIT_REPOSITORY ${LLVMGenXIntrinsics_GIT_REPO}
      GIT_TAG        ${LLVMGenXIntrinsics_GIT_TAG}
    )
    FetchContent_MakeAvailable(vc-intrinsics)
    FetchContent_GetProperties(vc-intrinsics)

    set(LLVMGenXIntrinsics_SOURCE_DIR ${vc-intrinsics_SOURCE_DIR})
    set(LLVMGenXIntrinsics_BINARY_DIR ${vc-intrinsics_BINARY_DIR})
  else (NOT DEFINED LLVMGenXIntrinsics_SOURCE_DIR)
    # -DLLVMGenXIntrinsics_SOURCE_DIR is provided
    message(STATUS "vc-intrinsics are added manually ${LLVMGenXIntrinsics_SOURCE_DIR}")

    set(LLVMGenXIntrinsics_BINARY_DIR ${CMAKE_BINARY_DIR}/vc-intrinsics-build)
    add_subdirectory(${LLVMGenXIntrinsics_SOURCE_DIR} ${LLVMGenXIntrinsics_BINARY_DIR})
  endif (NOT DEFINED LLVMGenXIntrinsics_SOURCE_DIR)

  target_include_directories(LLVMGenXIntrinsics
    PUBLIC $<BUILD_INTERFACE:${LLVMGenXIntrinsics_SOURCE_DIR}/GenXIntrinsics/include>
    PUBLIC $<BUILD_INTERFACE:${LLVMGenXIntrinsics_BINARY_DIR}/GenXIntrinsics/include>
  )
endif (NOT TARGET LLVMGenXIntrinsics)

set_property(GLOBAL PROPERTY LLVMGenXIntrinsics_SOURCE_PROP ${LLVMGenXIntrinsics_SOURCE_DIR})
set_property(GLOBAL PROPERTY LLVMGenXIntrinsics_BINARY_PROP ${LLVMGenXIntrinsics_BINARY_DIR})

add_llvm_component_library(LLVMSYCLLowerIR
  ESIMD/LowerESIMD.cpp
  ESIMD/LowerESIMDKernelProps.cpp
  ESIMD/LowerESIMDVLoadVStore.cpp
  ESIMD/LowerESIMDVecArg.cpp
  ESIMD/ESIMDUtils.cpp
  ESIMD/ESIMDVerifier.cpp
  ESIMD/LowerESIMDKernelAttrs.cpp
  LowerInvokeSimd.cpp
  LowerWGScope.cpp
  LowerWGLocalMemory.cpp
  MutatePrintfAddrspace.cpp
  SYCLPropagateAspectsUsage.cpp

  LocalAccessorToSharedMemory.cpp
  GlobalOffset.cpp
  TargetHelpers.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/SYCLLowerIR
  ${LLVM_MAIN_SRC_DIR}/projects/vc-intrinsics/GenXIntrinsics/include
  ${LLVM_BINARY_DIR}/projects/vc-intrinsics/GenXIntrinsics/include

  DEPENDS
  intrinsics_gen
  LLVMGenXIntrinsics
  LLVMDemangle
  LLVMTransformUtils

  LINK_LIBS
  LLVMGenXIntrinsics
  LLVMDemangle
  LLVMTransformUtils

  LINK_COMPONENTS
  Analysis
  Core
  Support
  )

target_include_directories(LLVMSYCLLowerIR
  PRIVATE ${LLVM_MAIN_SRC_DIR}/projects/vc-intrinsics/GenXIntrinsics/include
  PRIVATE ${LLVM_BINARY_DIR}/projects/vc-intrinsics/GenXIntrinsics/include)

## Remove the Werror for external component GenXIntrinsics.
if(MSVC)
  target_compile_options(LLVMGenXIntrinsics  PRIVATE "-WX-")
else()
  target_compile_options(LLVMGenXIntrinsics  PRIVATE "-Wno-error")
endif()
