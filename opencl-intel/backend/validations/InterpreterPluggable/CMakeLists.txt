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

use_rtti(FALSE)

set(TARGET_NAME LLVMInterpreterPluggable)

include_directories(${LLVM_INCLUDE_DIRS} ${BACKEND_ROOT_DIR}/name_mangling)

add_opencl_library(
  ${TARGET_NAME}
  STATIC
  EXCLUDE_FROM_ALL_BUILD
  OpenCLCompilationFlags.h
  OpenCLCompilationFlags.cpp
  InterpreterPluggable.h
  InterpreterPluggable.cpp
  WorkGroupBuiltinsNames.h
  WorkGroupBuiltinsNames.cpp
  InterpreterPlugIn.h)
# During the compilation of the LLVMInterpreter the Interpreter.h file is
# copying to the build directory (and then to the install one - but this is
# irrelevant for current problem). This file is then used by IntepreterPluggable
# build. So we need to ensure the LLVMInterpreter is running first
add_dependencies(${TARGET_NAME} LLVMInterpreter)
