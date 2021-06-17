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

    # FIXME: temporarily use a fork of intel/vc-intrinsics.git with getAnyName function
    # disabled as it is not compatible with https://reviews.llvm.org/D99173.
    if( INTEL_CUSTOMIZATION )
        # ef6c92cd is 8b31d10 with workaroudn for D99173, hosted on intel-innersource.
        set(LLVMGenXIntrinsics_GIT_TAG ef6c92cd3511e4eee638ce6ad0b957e5891ee764)
    else(INTEL_CUSTOMIZATION)
        set(LLVMGenXIntrinsics_GIT_TAG c3ecb918fc00913b0dc9b1bef1912df0361b74b0)
    endif(INTEL_CUSTOMIZATION)

    if( INTEL_CUSTOMIZATION )
      set( LLVMGenXIntrinsics_GIT_REPO_PATH ssh://git-amr-2.devtools.intel.com:29418 )
      #set( LLVMGenXIntrinsics_GIT_REPO_NAME dpd_icl-vc-intrinsics.git )
      # Temporarily use intel-innersource repo even for open-source
      # vc-intrinsics, to be able to use the workaround for D99173.
      set( LLVMGenXIntrinsics_GIT_REPO_NAME drivers.gpu.compute.vc-intrinsics.git )

# INTEL_FEATURE_ESIMD_EMBARGO
      if( IS_INTERNAL_ESIMD_BUILD )
        # use different git repo and tag for the internal ESIMD EMBARGO build
        set( LLVMGenXIntrinsics_GIT_REPO_NAME drivers.gpu.compute.vc-intrinsics.git )
        # f3164c49735fa3 is b0cf256acb08d8 with workaround for D99173.
        set( LLVMGenXIntrinsics_GIT_TAG f3164c49735fa322d4b2062fe144767e98b27ecf )
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
         https://github.com/abhinavgaba/vc-intrinsics.git )
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
