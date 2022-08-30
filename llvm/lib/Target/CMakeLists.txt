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
list(APPEND LLVM_COMMON_DEPENDS intrinsics_gen)

list(APPEND LLVM_TABLEGEN_FLAGS -I ${LLVM_MAIN_SRC_DIR}/lib/Target)

add_llvm_component_library(LLVMTarget
  Target.cpp
  TargetIntrinsicInfo.cpp
  TargetLoweringObjectFile.cpp
  TargetMachine.cpp
  TargetMachineC.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Target

  LINK_COMPONENTS
  Analysis
  Core
  MC
  Support
  )

# When building shared objects for each target there are some internal APIs
# that are used across shared objects which we can't hide.
if (NOT BUILD_SHARED_LIBS AND NOT APPLE AND
    (NOT (WIN32 OR CYGWIN) OR (MINGW AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")) AND
    NOT (${CMAKE_SYSTEM_NAME} MATCHES "AIX") AND
    NOT DEFINED CMAKE_CXX_VISIBILITY_PRESET)
  # Set default visibility to hidden, so we don't export all the Target classes
  # in libLLVM.so.
  set(CMAKE_CXX_VISIBILITY_PRESET hidden)
endif()

foreach(t ${LLVM_TARGETS_TO_BUILD})
  message(STATUS "Targeting ${t}")
  if(INTEL_CUSTOMIZATION)
# INTEL_FEATURE_CSA
    if("${t}" STREQUAL "CSA")
      is_intel_feature_enabled(p ${LLVM_INTELFEATURE_PREFIX}_CSA)
      if(p)
        add_subdirectory(Intel_${t})
        continue()
      else()
        message(FATAL_ERROR
          "Trying to build CSA target without CSA enabled in "
          "LLVM_INTEL_FEATURES.")
      endif()
    endif()
# end INTEL_FEATURE_CSA
  endif(INTEL_CUSTOMIZATION)
  add_subdirectory(${t})
endforeach()

# Currently we do not allow libraries from lib to reference targets directly.
# This property is used to enforce that convention. It is important because the
# logic in llvm_map_components_to_libnames is order dependent on the target
# libraries being created.
set_property(GLOBAL PROPERTY LLVM_TARGETS_CONFIGURED On)
