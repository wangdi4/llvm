# Lowering of SYCL ESIMD kernels depends on vc-intrinsics
# NOTE: could have been added earlier from llvm/projects
if (NOT TARGET LLVMGenXIntrinsics)
  if (NOT DEFINED LLVMGenXIntrinsics_SOURCE_DIR)

    set(LLVMGenXIntrinsics_GIT_TAG b831d10e49e1fb8fcd92b5b50e2cdc8f9cb6277b)

    if( INTEL_CUSTOMIZATION )
      set( IS_INTERNAL_ESIMD_BUILD FALSE )
# INTEL_FEATURE_ESIMD_EMBARGO
      # TODO the following line is commented out to w/a CMPLRLLVM-27163 -
      # this reverts xisa build to use public vc-intrinsics repo. This will
      # make ESIMD tests using PVC features fail for now.
      #is_intel_feature_enabled(IS_INTERNAL_ESIMD_BUILD ${LLVM_INTELFEATURE_PREFIX}_ESIMD_EMBARGO)
# end INTEL_FEATURE_ESIMD_EMBARGO

      set( LLVMGenXIntrinsics_GIT_REPO_PATH ssh://git-amr-2.devtools.intel.com:29418 )
      set( LLVMGenXIntrinsics_GIT_REPO_NAME dpd_icl-vc-intrinsics.git )

# INTEL_FEATURE_ESIMD_EMBARGO
      if( IS_INTERNAL_ESIMD_BUILD )
        # use different git repo and tag for the internal ESIMD EMBARGO build
        set( LLVMGenXIntrinsics_GIT_REPO_NAME drivers.gpu.compute.vc-intrinsics.git )
        set( LLVMGenXIntrinsics_GIT_TAG 4dec1a45eaa935adbb0a60fae99344e27c2c7b24 )
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
  else()
    # -DLLVMGenXIntrinsics_SOURCE_DIR is provided
    message(STATUS "vc-intrinsics are added manually ${LLVMGenXIntrinsics_SOURCE_DIR}")

    set(LLVMGenXIntrinsics_BINARY_DIR ${CMAKE_BINARY_DIR}/vc-intrinsics-build)
    add_subdirectory(${LLVMGenXIntrinsics_SOURCE_DIR} ${LLVMGenXIntrinsics_BINARY_DIR})
  endif()

  target_include_directories(LLVMGenXIntrinsics
    PUBLIC $<BUILD_INTERFACE:${LLVMGenXIntrinsics_SOURCE_DIR}/GenXIntrinsics/include>
    PUBLIC $<BUILD_INTERFACE:${LLVMGenXIntrinsics_BINARY_DIR}/GenXIntrinsics/include>
  )
endif()

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
