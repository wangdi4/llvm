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

set(TARGET_NAME reflection_module)

project(${TARGET_NAME})

# enables compound initialization literals
remove_definitions("-pedantic")

if(BACKEND_BUILD_VERIFICATION_LIB)
  add_subdirectory(unittest)
endif(BACKEND_BUILD_VERIFICATION_LIB)

set(BUILT_IN_LIST_INC ${OCL_BINARY_DIR}/include/BuiltinList.inc)
add_custom_command(
  OUTPUT ${BUILT_IN_LIST_INC}
  COMMAND
    ocl-tblgen -gen-ocl-biheader -I
    ${OCL_SOURCE_DIR}/backend/libraries/ocl_builtins
    ${OCL_SOURCE_DIR}/backend/libraries/ocl_builtins/source/misc_main.td -o
    ${BUILT_IN_LIST_INC}
  DEPENDS ocl-tblgen clang ${OCL_ALL_TD_FILES}
  COMMENT "Generating demangled headers..."
  VERBATIM)
add_custom_target(builtin_list_target DEPENDS ${BUILT_IN_LIST_INC})

set(TARGET_SOURCE_FILES Mangler.cpp BuiltinKeeper.cpp VersionStrategy.cpp
                        BuiltinMap.cpp)

set(TARGET_INCLUDE_FILES Mangler.h BuiltinKeeper.h VersionStrategy.h
                         ${BUILT_IN_LIST_INC} BuiltinMap.h utils.h)

add_opencl_library(${TARGET_NAME} STATIC ${TARGET_SOURCE_FILES}
                   ${TARGET_INCLUDE_FILES})

include_directories(
  ${LLVM_INCLUDE_DIRS} ${OCL_SOURCE_DIR}/backend/name_mangling
  ${OCL_SOURCE_DIR}/backend/utils ${OCL_BINARY_DIR}/include)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_dependencies(${TARGET_NAME} builtin_list_target)

set_target_properties(${TARGET_NAME}
                      PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/reflection)
