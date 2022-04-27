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
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )

if(NOT DEFINED ITTAPI_GIT_REPOSITORY)
  if(INTEL_CUSTOMIZATION)
    set(ITTAPI_GIT_REPOSITORY https://github.com/intel-innersource/applications.compilers.source.ittapi)
    if( DEFINED ENV{ICS_GIT_MIRROR} AND NOT "$ENV{ICS_GIT_MIRROR}" STREQUAL "" )
      set(ITTAPI_GIT_REPOSITORY  $ENV{ICS_GIT_MIRROR}/dpd_icl-ittapi)
      STRING(REGEX REPLACE "\\\\" "/" ITTAPI_GIT_REPOSITORY "${ITTAPI_GIT_REPOSITORY}")
    endif()
  else (INTEL_CUSTOMIZATION)
    set(ITTAPI_GIT_REPOSITORY https://github.com/intel/ittapi.git)
  endif (INTEL_CUSTOMIZATION)
endif()

if(NOT DEFINED ITTAPI_GIT_TAG)
    set(ITTAPI_GIT_TAG v3.18.12)
endif()

if(NOT DEFINED ITTAPI_SOURCE_DIR)
    set(ITTAPI_SOURCE_DIR ${PROJECT_BINARY_DIR})
endif()

if(NOT EXISTS ${ITTAPI_SOURCE_DIR}/ittapi)
if(INTEL_CUSTOMIZATION)
    execute_process(COMMAND ${GIT_EXECUTABLE} clone ${ITTAPI_GIT_REPOSITORY} ittapi
                    WORKING_DIRECTORY ${ITTAPI_SOURCE_DIR}
                    RESULT_VARIABLE GIT_CLONE_RESULT)
else(INTEL_CUSTOMIZATION)
    execute_process(COMMAND ${GIT_EXECUTABLE} clone ${ITTAPI_GIT_REPOSITORY}
                    WORKING_DIRECTORY ${ITTAPI_SOURCE_DIR}
                    RESULT_VARIABLE GIT_CLONE_RESULT)
endif(INTEL_CUSTOMIZATION)
    if(NOT GIT_CLONE_RESULT EQUAL "0")
        message(FATAL_ERROR "git clone ${ITTAPI_GIT_REPOSITORY} failed with ${GIT_CLONE_RESULT}, please clone ${ITTAPI_GIT_REPOSITORY}")
    endif()
endif()

execute_process(COMMAND ${GIT_EXECUTABLE} checkout ${ITTAPI_GIT_TAG}
                WORKING_DIRECTORY ${ITTAPI_SOURCE_DIR}/ittapi
                RESULT_VARIABLE GIT_CHECKOUT_RESULT)
if(NOT GIT_CHECKOUT_RESULT EQUAL "0")
    message(FATAL_ERROR "git checkout ${ITTAPI_GIT_TAG} failed with ${GIT_CHECKOUT_RESULT}, please checkout ${ITTAPI_GIT_TAG} at ${ITTAPI_SOURCE_DIR}/ittapi")
endif()

include_directories( ${ITTAPI_SOURCE_DIR}/ittapi/include/ )

if( HAVE_LIBDL )
    set(LLVM_INTEL_JIT_LIBS ${CMAKE_DL_LIBS})
endif()

set(LLVM_INTEL_JIT_LIBS ${LLVM_PTHREAD_LIB} ${LLVM_INTEL_JIT_LIBS})


add_llvm_component_library(LLVMIntelJITEvents
  IntelJITEventListener.cpp
  jitprofiling.c
  ${ITTAPI_SOURCE_DIR}/ittapi/src/ittnotify/ittnotify_static.c

  DEPENDS
  LLVMCodeGen

  LINK_LIBS ${LLVM_INTEL_JIT_LIBS}

  LINK_COMPONENTS
  CodeGen
  Core
  DebugInfoDWARF
  Support
  Object
  ExecutionEngine
)
