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


set(TARGET tbb_native_test_type)

include_directories(BEFORE ${OCL_SOURCE_DIR}/externals/tbb/enhancements)

add_definitions(-DTBB_PREVIEW_LOCAL_OBSERVER=1 -DTBB_PREVIEW_TASK_ARENA=1
                -D__TBB_NO_IMPLICIT_LINKAGE=1 -D__TBB_EXTRA_DEBUG=1)

find_sources(${IGNORE_FILES})
calculate_target_sources()

add_ocl_unittest(${TARGET} SOURCE_FILES ${TARGET_SOURCES})

if(WIN32)
  # This is a fix due to a bug in CMake, Does not add the flag /DEBUG to the
  # linker flags in Release mode. The /DEBUG flag is required in order to create
  # stripped pdbs.
  set_target_properties(
    ${TARGET}
    PROPERTIES
      LINK_FLAGS_RELEASE
      "${LINK_FLAGS_RELEASE} /DEBUG")
  list(APPEND LINK_LIBS delayimp.lib)
else(WIN32)
  if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set_property(
      TARGET ${TARGET}
      APPEND_STRING
      PROPERTY LINK_FLAGS " -pie")
  endif(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
endif(WIN32)

# gtest depends on LLVMSupport
list(APPEND LINK_LIBS LLVMSupport)

target_link_libraries(${TARGET} PRIVATE ${LINK_LIBS})

link_target_with_tbb_library(${TARGET})

add_dependencies(${TARGET} ${OCL_RT_TEST_DEPS})
