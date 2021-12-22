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

    # Author: Aleksander Us <aleksander.us@intel.com>
    # Date:   Mon Oct 18 18:03:24 2021 +0000
    #     Wrap arg_size for call instructions
    set(LLVMGenXIntrinsics_GIT_TAG 6a7e93d3d90638b6565d97cef96ffaf0f77d688c)

    if (INTEL_CUSTOMIZATION)
      # for xmain builds the LLVMGenXIntrinsics_GIT_REPO is constructed from
      # GIT_REPO_PATH and GIT_REPO_TAG set independently depending on build
      # configuration (public/embargo) and condition (building from under ICS
      # or not)

      # default repo path for public xmain builds, not using ICS
      set(LLVMGenXIntrinsics_GIT_REPO_PATH https://github.com/intel-innersource)
      set(LLVMGenXIntrinsics_GIT_REPO_NAME applications.compilers.source.vc-intrinsics.git)

      # Author:     Aleksander Us <aleksander.us@intel.com>
      # AuthorDate: Mon Oct 18 18:03:24 2021 +0000
      # Commit:     Alexander Us <aleksander.us@intel.com>
      #     Wrap arg_size for call instructions
      set(LLVMGenXIntrinsics_GIT_TAG 6a7e93d3d90638b6565d97cef96ffaf0f77d688c)

      # Determine if the build is embargo and provide specific settings.
      set(IS_INTERNAL_ESIMD_BUILD FALSE)
# INTEL_FEATURE_ESIMD_EMBARGO
      is_intel_feature_enabled(IS_INTERNAL_ESIMD_BUILD ${LLVM_INTELFEATURE_PREFIX}_ESIMD_EMBARGO)

      if (IS_INTERNAL_ESIMD_BUILD)
        # override git repo name and tag for the internal ESIMD EMBARGO build
        set(LLVMGenXIntrinsics_GIT_REPO_NAME drivers.gpu.compute.vc-intrinsics.git)

        # Author: Dmitry Bushev <dmitry.bushev@intel.com>
        # Date:   Mon Nov 1 15:23:45 2021 +0300
        #    Fixed SPIRVWriterAdaptor pass not adding attribute on address convert intrinsic (#143)
        set(LLVMGenXIntrinsics_GIT_TAG 10163a5d7ed1a0d6f01a5d3dc432660ed486fb86)

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
  LowerWGScope.cpp
  LowerESIMD.cpp
  LowerESIMDVLoadVStore.cpp
  LowerESIMDVecArg.cpp
  LowerWGLocalMemory.cpp
  ESIMDVerifier.cpp
  MutatePrintfAddrspace.cpp

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
