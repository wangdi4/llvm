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
add_llvm_component_library(LLVMInterpreter
  Execution.cpp
  ExternalFunctions.cpp
  Interpreter.cpp

  DEPENDS
  intrinsics_gen

  LINK_COMPONENTS
  CodeGen
  Core
  ExecutionEngine
  Support
  )

if( LLVM_ENABLE_FFI )
  target_link_libraries( LLVMInterpreter PRIVATE FFI::ffi )
endif()

if (INTEL_CUSTOMIZATION)
  configure_file(
    Interpreter.h
    ${LLVM_BINARY_DIR}/include/llvm/ExecutionEngine/Interpreter/Interpreter.h)

  install(FILES Interpreter.h DESTINATION include/llvm/ExecutionEngine/Interpreter)
endif (INTEL_CUSTOMIZATION)
