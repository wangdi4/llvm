# Copyright (C) 2022 Intel Corporation
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

use_eh(TRUE)

set(TARGET_NAME ${OCL_TBLGEN})

add_definitions(
  -D CCLANG_INCLUDE_PATH=${CCLANG_DEV_INCLUDE_DIRS}
  -D CLANG_BIN_PATH=${CLANG_BIN_PATH}
  -D CL_BUILTIN_SOURCE_DIR=${OCL_SOURCE_DIR}/backend/libraries/ocl_builtins
)

include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${CL_API_HEADERS}
  ${OCL_SOURCE_DIR}/backend/dynamic_lib
  ${OCL_SOURCE_DIR}/backend/validations/OCLBuilder/
  ${OCL_SOURCE_DIR}/backend/validations/Common/
  ${OCL_SOURCE_DIR}/backend/name_mangling/
  ${OCL_SOURCE_DIR}/cl_api
)

if (OPENCL_INTREE_BUILD)
  llvm_map_components_to_libnames(LLVM_MODULE_LIBS
    Analysis
    Core
    IRReader
    Support
    TableGen
    Intel_DPCPPKernelTransforms
  )
endif()

add_opencl_executable(${TARGET_NAME}
  OclBuiltinEmitter.cpp
  OclBuiltinsHeaderGen.cpp
  VectInfoGenerator.cpp
  TableGen.cpp
  CodeFormatter.cpp
  ConversionParser.cpp
  ClangUtils.cpp

  LINK_LIBS
  ${LLVM_MODULE_LIBS}
)

if (OPENCL_INTREE_BUILD)
  add_dependencies(${TARGET_NAME} intrinsics_gen)
  # ocl-tblgen executes clang, so clang has to be available
  # before we can launch ocl-tblgen. Instead of adding
  # the clang dependece to all target invoking ocl-tblgen,
  # add clang dependency here.
  add_dependencies(${TARGET_NAME} ${CLANG})
endif()

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME} )
