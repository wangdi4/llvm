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

#
# Project variables:
#  OCL_SOURCE_DIR    - points to the top src directory
#  OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root
#                      of generated files
#

if (NOT WIN32)
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -D_FORTIFY_SOURCE=2")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}   -D_FORTIFY_SOURCE=2")
else (NOT WIN32)
  # Common Runtime has to use deprecated OpenCL API functions.
  # Suppress 'function': was declared deprecated'
  add_definitions(-wd4996)
endif(NOT WIN32)

set(TARGET_NAME "intelopencl${ADDR}")

if (WIN32 AND (DEFINED ENV{DXSDK_DIR}))
  add_definitions (-DCL_DX9_MEDIA_SHARING_INTEL_EXT)
endif()

add_subdirectory(utils)

if (WIN32)
  copy_to(register_vendor32.reg
          register_vendor64.reg
          unregister_vendor32.reg
          unregister_vendor64.reg
          DESTINATION lib)

  install_to(register_vendor32.reg
             register_vendor64.reg
             unregister_vendor32.reg
             unregister_vendor64.reg
             DESTINATION lib
             COMPONENT ocl-common-runtime)
endif (WIN32)

FIND_SOURCES()
CALCULATE_TARGET_SOURCES()

# add a target named ${TARGET_NAME}
add_opencl_library (${TARGET_NAME} SHARED
                    ${TARGET_SOURCES}

                    INCLUDE_DIRS
                    ${CMAKE_CURRENT_SOURCE_DIR}/utils
                    ${CL_API_HEADERS}
                    ${CMAKE_CURRENT_SOURCE_DIR}
                    ${OCL_BINARY_DIR}

                    RC_TEMPLATE
                    ${CMAKE_CURRENT_SOURCE_DIR}/common_runtime.rc.in
)

if (NOT WIN32 AND NOT ANDROID)
  target_link_libraries(${TARGET_NAME} pthread)
endif (NOT WIN32 AND NOT ANDROID)
target_link_libraries(${TARGET_NAME} common_runtime_utils)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER common_runtime)

if (NOT WIN32)
  set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS -Wl,-Bsymbolic)
endif (NOT WIN32)

if (WIN32)
  file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${TARGET_NAME}.pdb PDB_NAME)
  set_target_properties(${TARGET_NAME} PROPERTIES
                        LINK_FLAGS_DEBUG "${LINK_FLAGS_DEBUG} /PDB:${PDB_NAME}"
                        LINK_FLAGS_RELEASE "${LINK_FLAGS_RELEASE} /PDB:${PDB_NAME}"
endif(WIN32)
