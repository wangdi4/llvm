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

set(TARGET_NAME ref_cl_builtins)

# remember OCL builtins source dir
set(REF_CL_BUILTIN_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# TODO: enable this warning later.
# There is such warning since llvm::GenericValue is used as return type of C
# function.
add_compile_options("-Wno-return-type-c-linkage")

# ------------------- Utility functions ---------------------------
function(tblgenrefoclblt OUTPUT_VAR OUTPUT_DIR IFN)
  string(REPLACE ".td" ".cpp" OFN ${IFN})
  add_custom_command(
    OUTPUT ${OUTPUT_DIR}/${OFN}
    COMMAND ref-ocl-tblgen ${ARGN} -I=${REF_CL_BUILTIN_SOURCE_DIR} -o
            ${OUTPUT_DIR}/${OFN} ${REF_CL_BUILTIN_SOURCE_DIR}/${IFN}
    DEPENDS ref-ocl-tblgen ${IFN}
    COMMENT "Generating reference built-in file ${OFN}...")
  set_source_files_properties(${OUTPUT_DIR}/${OFN} PROPERTIES GENERATED 1)
  set(${OUTPUT_VAR}
      ${OUTPUT_DIR}/${OFN}
      PARENT_SCOPE)
endfunction(tblgenrefoclblt)

# ------------------ Main -----------------------

list(
  APPEND
  TD_FILES
  BLTAsyncCopiesAndPrefetch.td
  BLTAtomic.td
  BLTCommon.td
  BLTConversionsChar.td
  BLTConversionsShort.td
  BLTConversionsInt.td
  BLTConversionsLong.td
  BLTConversionsFPoint.td
  BLTExplMemFenceOps.td
  BLTGeometric.td
  BLTImages.td
  BLTInteger.td
  BLTMath.td
  BLTMiscellaneousVector.td
  BLTRelational.td
  BLTWorkGroup.td
  BLTWorkItem.td
  VLoadStore.td)

list(
  APPEND
  OCL_INCLUDE_FILES
  BLTAsyncCopiesAndPrefetch.h
  BLTAtomic.h
  BLTCommon.h
  BLTConversion.h
  BLTExplMemFenceOps.h
  BLTGeometric.h
  BLTImages.h
  BLTInteger.h
  BLTMath.h
  BLTMiscellaneousVector.h
  BLTRelational.h
  BLTWorkGroup.h
  BLTWorkItem.h
  VLoadStore.h)

foreach(FILE ${TD_FILES})
  tblgenrefoclblt(OCL_TABLEGEN_SOURCE ${CMAKE_CURRENT_BINARY_DIR} ${FILE}
                  -gen-ocl-src)
  list(APPEND OCL_SOURCE_FILES ${OCL_TABLEGEN_SOURCE})
endforeach(FILE)

add_custom_target(
  ${TARGET_NAME}
  DEPENDS ${OCL_SOURCE_FILES} ref-ocl-tblgen
  COMMENT "Target ${TARGET_NAME} build completed"
  VERBATIM
  SOURCES ${OCL_SOURCE_FILES})

add_opencl_library(
  OCLBuiltinsRef
  STATIC
  EXCLUDE_FROM_ALL_BUILD
  ${OCL_SOURCE_FILES}
  ${OCL_INCLUDE_FILES}
  Helpers.h
  Helpers.cpp
  OCLBuiltinParser.h
  OCLBuiltinParser.cpp
  IWorkItemBuiltins.h
  INCLUDE_DIRS
  ${LLVM_INCLUDE_DIRS}
  ${CL_API_HEADERS}
  ${BACKEND_ROOT_DIR}/name_mangling
  ${VALIDATIONS_ROOT}/Common
  ${VALIDATIONS_ROOT}/REFALU
  ${REF_CL_BUILTIN_SOURCE_DIR}
  LINK_LIBS
  LLVMSYCLTransforms
  REFALU)

add_dependencies(OCLBuiltinsRef ${TARGET_NAME})

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "Backend")
