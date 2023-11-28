# INTEL_CUSTOMIZATION
#
# INTEL CONFIDENTIAL
#
# Modifications, Copyright (C) 2023 Intel Corporation
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

include(CheckCXXCompilerFlag)
include(CompilerRTCompile)
include(CompilerRTLink)

# INTEL_CUSTOMIZATION
# Remove libimf dependency using this flag
append_string_if(COMPILER_RT_HAS_INTEL_NO_INTEL_LIB_FLAG ${INTEL_NO_INTEL_LIB_FLAG} CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
# end INTEL_CUSTOMIZATION

set(MEMPROF_UNITTEST_CFLAGS
  ${COMPILER_RT_UNITTEST_CFLAGS}
  ${COMPILER_RT_GTEST_CFLAGS}
  ${COMPILER_RT_GMOCK_CFLAGS}
  ${SANITIZER_TEST_CXX_CFLAGS}
  -I${COMPILER_RT_SOURCE_DIR}/lib/
  -DSANITIZER_COMMON_NO_REDEFINE_BUILTINS
  -O2
  -g
  -fno-rtti
  -Wno-pedantic
  -fno-omit-frame-pointer)

# Suppress warnings for gmock variadic macros for clang and gcc respectively.
append_list_if(SUPPORTS_GNU_ZERO_VARIADIC_MACRO_ARGUMENTS_FLAG -Wno-gnu-zero-variadic-macro-arguments MEMPROF_UNITTEST_CFLAGS)
append_list_if(COMPILER_RT_HAS_WVARIADIC_MACROS_FLAG -Wno-variadic-macros MEMPROF_UNITTEST_CFLAGS)

file(GLOB MEMPROF_HEADERS ../*.h)

set(MEMPROF_SOURCES
  ../memprof_mibmap.cpp
  ../memprof_rawprofile.cpp)

set(MEMPROF_UNITTESTS
  rawprofile.cpp
  driver.cpp)

include_directories(../../../include)

set(MEMPROF_UNIT_TEST_HEADERS
  ${MEMPROF_HEADERS})

set(MEMPROF_UNITTEST_LINK_FLAGS
  ${COMPILER_RT_UNITTEST_LINK_FLAGS})

if(NOT WIN32)
  list(APPEND MEMPROF_UNITTEST_LINK_FLAGS -pthread)
endif()

set(MEMPROF_UNITTEST_LINK_LIBRARIES
  ${COMPILER_RT_UNWINDER_LINK_LIBS}
  ${SANITIZER_TEST_CXX_LIBRARIES})
list(APPEND MEMPROF_UNITTEST_LINK_LIBRARIES "dl")

if(COMPILER_RT_DEFAULT_TARGET_ARCH IN_LIST MEMPROF_SUPPORTED_ARCH)
  # MemProf unit tests are only run on the host machine.
  set(arch ${COMPILER_RT_DEFAULT_TARGET_ARCH})

  add_executable(MemProfUnitTests 
    ${MEMPROF_UNITTESTS}
    ${COMPILER_RT_GTEST_SOURCE}
    ${COMPILER_RT_GMOCK_SOURCE}
    ${MEMPROF_SOURCES}
    $<TARGET_OBJECTS:RTSanitizerCommon.${arch}>
    $<TARGET_OBJECTS:RTSanitizerCommonCoverage.${arch}>
    $<TARGET_OBJECTS:RTSanitizerCommonLibc.${arch}>
    $<TARGET_OBJECTS:RTSanitizerCommonSymbolizer.${arch}>
    $<TARGET_OBJECTS:RTSanitizerCommonSymbolizerInternal.${arch}>)
  set_target_compile_flags(MemProfUnitTests ${MEMPROF_UNITTEST_CFLAGS})
  set_target_link_flags(MemProfUnitTests ${MEMPROF_UNITTEST_LINK_FLAGS})
  target_link_libraries(MemProfUnitTests ${MEMPROF_UNITTEST_LINK_LIBRARIES})

  if (TARGET cxx-headers OR HAVE_LIBCXX)
    add_dependencies(MemProfUnitTests cxx-headers)
  endif()

  set_target_properties(MemProfUnitTests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif()
