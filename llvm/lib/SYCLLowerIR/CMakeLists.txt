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
  if( INTEL_CUSTOMIZATION )
    set( IS_INTERNAL_ESIMD_BUILD FALSE )
# INTEL_FEATURE_ESIMD_EMBARGO
    is_intel_feature_enabled(IS_INTERNAL_ESIMD_BUILD ${LLVM_INTELFEATURE_PREFIX}_ESIMD_EMBARGO)

    if( IS_INTERNAL_ESIMD_BUILD )
      add_compile_definitions(__INTEL_EMBARGO__)
      # LLVM_CMAKE_DIR is required by the internal vc-intrinsics repo, but is
      # not set for non-standalone builds. Use LLVM_CMAKE_PATH instead.
      if (NOT LLVM_CMAKE_DIR)
        set(LLVM_CMAKE_DIR ${LLVM_CMAKE_PATH})
      endif()
    endif()
# end INTEL_FEATURE_ESIMD_EMBARGO
  endif()

  if (NOT DEFINED LLVMGenXIntrinsics_SOURCE_DIR)
    set(LLVMGenXIntrinsics_GIT_REPO https://github.com/intel/vc-intrinsics.git)

    # Author: Aleksander Us <aleksander.us@intel.com>
    # Date:   Mon Oct 18 18:03:24 2021 +0000
    #     Wrap arg_size for call instructions
    set(LLVMGenXIntrinsics_GIT_TAG 6a7e93d3d90638b6565d97cef96ffaf0f77d688c)

    set( LLVMGenXIntrinsics_GIT_REPO_PATH https://github.com/intel-innersource )
    set( LLVMGenXIntrinsics_GIT_REPO_NAME drivers.gpu.compute.vc-intrinsics.git )
    set( LLVMGenXIntrinsics_GIT_TAG dpcpp_staging )

    if( INTEL_CUSTOMIZATION )
# INTEL_FEATURE_ESIMD_EMBARGO
      if( IS_INTERNAL_ESIMD_BUILD )
        # use different git tag for the internal ESIMD EMBARGO build
        set( LLVMGenXIntrinsics_GIT_TAG fbfb199f83f74a838efe363202dbe666df83a2c9 )
      endif()
# end INTEL_FEATURE_ESIMD_EMBARGO

      if( DEFINED ENV{ICS_GIT_MIRROR} AND NOT "$ENV{ICS_GIT_MIRROR}" STREQUAL "" )
        set( LLVMGenXIntrinsics_GIT_REPO_PATH $ENV{ICS_GIT_MIRROR} )
        STRING(REGEX REPLACE "\\\\" "/" LLVMGenXIntrinsics_GIT_REPO_PATH "${LLVMGenXIntrinsics_GIT_REPO_PATH}")
# INTEL_FEATURE_ESIMD_EMBARGO
      elseif( IS_INTERNAL_ESIMD_BUILD )
        message( FATAL_ERROR "xmain ESIMD_EMBARGO build requires ICS_GIT_MIRROR env var to be set" )
# end INTEL_FEATURE_ESIMD_EMBARGO
      endif()

      set( LLVMGenXIntrinsics_GIT_REPO
        ${LLVMGenXIntrinsics_GIT_REPO_PATH}/${LLVMGenXIntrinsics_GIT_REPO_NAME} )
    else(INTEL_CUSTOMIZATION)
      set( LLVMGenXIntrinsics_GIT_REPO
         https://github.com/intel/vc-intrinsics.git )
    endif(INTEL_CUSTOMIZATION)

    message(STATUS "vc-intrinsics are missing. Will try to download them from ${LLVMGenXIntrinsics_GIT_REPO} ") #INTEL_CUSTOMIZATION
    include(FetchContent)
    FetchContent_Declare(vc-intrinsics
      GIT_REPOSITORY ${LLVMGenXIntrinsics_GIT_REPO} #INTEL_CUSTOMIZATION
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
