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

# The conformance tests must be compiled with the headers located in
# ${OCL_CONFORMANCE_INCLUDE} and not the ones in ${OCL_GLOBAL_INCLUDE}
# Therefore, it must be added to the include directories only after the
# conformance subdirectory has been added.
include_directories(BEFORE ${OCL_GLOBAL_INCLUDE})

# some tests have to use deprecated OpenCL API functions.
if(WIN32)
  add_definitions(-wd4996 # Suppress 'function': was declared deprecated'
  )
endif(WIN32)

if(NOT DEFINED LLVM_PATH_FE)
  message(
    FATAL_ERROR
      "LLVM_PATH_FE is not specified. Please specify LLVM library location for frontend using LLVM_PATH_FE parameter to CMAKE"
  )
endif()

if(NOT OPENCL_INTREE_BUILD)
  set(LLVM_PATH ${LLVM_PATH_FE})
  find_package(LLVM REQUIRED)
  set(CLANG "${LLVM_BINARY_DIR}/clang")
else()
  set(CLANG clang)
endif()

set(SPIRV_CONVERTER "${LLVM_PATH_BE}/bin/llvm-spirv")
set(GTEST_INCLUDE_DIR "${LLVM_MAIN_SRC_DIR}/utils/unittest/googletest/include")
set(CLANG_LIBRARY_DIR
    "${LLVM_LIBRARY_DIR}/clang/${CLANG_VERSION}/lib/${LLVM_TARGET_TRIPLE}")

include(itt)

# LIT infrastructure configuration
if(NOT OPENCL_INTREE_BUILD)
  include(${LLVM_PATH}/lib/cmake/llvm/AddLLVM.cmake)
endif()

# Compile .cl to .bc
macro(compile_bitcode SOURCE OUTPUT)
  cmake_parse_arguments("ARG" "BUILD_INV" "" "EXTRA_CLANG_FLAGS" ${ARGN})
  if((BUILD_X64 AND (NOT ARG_BUILD_INV)) OR ((NOT BUILD_X64) AND ARG_BUILD_INV))
    set(SPIR_OPTIONS -triple spir64-unknown-unknown -D__x86_64__)
  else()
    set(SPIR_OPTIONS -triple spir-unknown-unknown -D__i386__)
  endif()
  add_custom_command(
    OUTPUT ${OUTPUT}
    COMMAND
      ${CLANG} -cc1 -x cl ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE}
      -I${CLANG_OPENCL_HEADERS_DIR} -emit-llvm-bc ${SPIR_OPTIONS}
      -disable-llvm-passes -disable-intel-proprietary-opts -include opencl-c.h
      ${ARG_EXTRA_CLANG_FLAGS} -no-opaque-pointers -o
      ${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT}
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE}
    DEPENDS ${CLANG_OPENCL_HEADERS_DIR}/opencl-c.h ${CLANG}
    COMMENT "Generate LLVM bitcode file ${OUTPUT}"
    VERBATIM)
endmacro(compile_bitcode)

# Compile .cl to SPIR-V
macro(compile_spv SOURCE OUTPUT)
  cmake_parse_arguments("ARG" "BUILD_INV" "BC_OUTPUT_FILE" "EXTRA_CLANG_FLAGS"
                        ${ARGN})
  if(NOT ARG_BC_OUTPUT_FILE)
    get_filename_component(SOURCE_NAME ${SOURCE} NAME)
    string(REGEX REPLACE ".cl$" ".bc" BC_OUTPUT_FILE ${SOURCE_NAME})
  else()
    set(BC_OUTPUT_FILE ${ARG_BC_OUTPUT_FILE})
  endif()
  if(ARG_BUILD_INV)
    set(BUILD_INV BUILD_INV)
  endif()
  compile_bitcode(${SOURCE} ${BC_OUTPUT_FILE} EXTRA_CLANG_FLAGS
                  ${ARG_EXTRA_CLANG_FLAGS} ${BUILD_INV})
  add_custom_command(
    OUTPUT ${OUTPUT}
    COMMAND ${SPIRV_CONVERTER} -o ${OUTPUT}
            ${CMAKE_CURRENT_BINARY_DIR}/${BC_OUTPUT_FILE}
    DEPENDS ${SPIRV_CONVERTER} ${BC_OUTPUT_FILE}
    COMMENT "Generate spirv binary file ${OUTPUT}"
    VERBATIM)
endmacro(compile_spv)

# It is used by lit.cfg to find lit.site.cfg when user launches LIT tests from a
# certain sub-directory instead of launching 'check-ocl-runtime' target
set(RT_TEST_PARAMS rt_lit_site_config=${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg)

# Use the same args as LLVM
set(RT_TEST_ARGS
    ${LLVM_LIT_ARGS}
    CACHE STRING "Options for LIT test")

set(RT_TESTS_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

get_property(OCL_BUILTIN_LIBS GLOBAL PROPERTY OCL_BUILTIN_LIBS)
set(OCL_RT_TEST_DEPS ${OCL_BUILTIN_LIBS})

# We need to pass some cmake variables into lit .cfg scripts
configure_lit_site_cfg(${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.in
                       ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg)

# An umbrella target for all OpenCL runtime tests.
umbrella_lit_testsuite_begin(check-ocl-runtime)

# Unittests
add_subdirectory(test_utils)
include_directories(test_utils/export)

if(NOT ${ADDR} STREQUAL "32")
  add_subdirectory(fpga_test_type)
endif(NOT ${ADDR} STREQUAL "32")
add_subdirectory(tbb_native_test_type)
add_subdirectory(cpu_device_test_type)
add_subdirectory(framework_test_type)
add_subdirectory(FrontendDriver_test_type)
add_subdirectory(task_executor_test_type)
add_subdirectory(bi_test_type)
add_subdirectory(backend_test_type)
add_subdirectory(name_mangling_test_type)
add_subdirectory(metadata_api_test_type)
add_subdirectory(subgroup_test_type)

if(USE_VALGRIND)
  add_subdirectory(valgrind)
endif(USE_VALGRIND)

if(INCLUDE_CMRT)
  message(STATUS "Adding Common runtime tests ...")
  if(NOT BUILD_X64)
    add_subdirectory(common_runtime_test_type) # until compilation errors are
                                               # resolved
  endif(NOT BUILD_X64)
endif(INCLUDE_CMRT)

umbrella_lit_testsuite_end(check-ocl-runtime)

set_target_properties(check-ocl-runtime PROPERTIES FOLDER "Tests")
