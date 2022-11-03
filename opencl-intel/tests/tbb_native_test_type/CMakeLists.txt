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

link_directories(${TBB_LIB_DIR})

add_ocl_unittest(${TARGET} SOURCE_FILES ${TARGET_SOURCES})

if(WIN32)
  # This is a fix due to a bug in CMake, Does not add the flag /DEBUG to the
  # linker flags in Release mode. The /DEBUG flag is required in order to create
  # stripped pdbs.
  set_target_properties(
    ${TARGET}
    PROPERTIES
      LINK_FLAGS_DEBUG
      "${LINK_FLAGS_DEBUG} tbbmalloc_debug.lib tbb12_debug${TBB_BINARIES_POSTFIX}.lib /DELAYLOAD:tbb12_debug${TBB_BINARIES_POSTFIX}.dll"
  )
  set_target_properties(
    ${TARGET}
    PROPERTIES
      LINK_FLAGS_RELEASE
      "${LINK_FLAGS_RELEASE} tbbmalloc.lib tbb12${TBB_BINARIES_POSTFIX}.lib /DELAYLOAD:tbb12${TBB_BINARIES_POSTFIX}.dll /DEBUG")
  list(APPEND LINK_LIBS delayimp.lib)
else(WIN32)
  if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    list(APPEND LINK_LIBS ${IMPLIB_PREFIX}tbb_debug${IMPLIB_SUFFIX})
    list(APPEND LINK_LIBS ${IMPLIB_PREFIX}tbbmalloc_debug${IMPLIB_SUFFIX})
    set_property(
      TARGET ${TARGET}
      APPEND_STRING
      PROPERTY LINK_FLAGS " -pie")
  else()
    list(APPEND LINK_LIBS ${IMPLIB_PREFIX}tbb${IMPLIB_SUFFIX})
    list(APPEND LINK_LIBS ${IMPLIB_PREFIX}tbbmalloc${IMPLIB_SUFFIX})
  endif(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
endif(WIN32)

# gtest depends on LLVMSupport
list(APPEND LINK_LIBS LLVMSupport)

target_link_libraries(${TARGET} ${LINK_LIBS})

add_dependencies(${TARGET} ${OCL_RT_TEST_DEPS})
