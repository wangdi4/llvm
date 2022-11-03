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

set(TARGET_NAME_PREFIX ${OCL_LIBRARY_TARGET_NAME})
project(${TARGET_NAME_PREFIX})

set(RTL_TARGET_NAME ${TARGET_NAME_PREFIX})
set(INTERMEDIATE_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
# target directory for OCL .rtl and .o files
set(LIBRARY_OUTPUT_DIR ${OCL_OUTPUT_LIBRARY_DIR})

function(create_ocl_kernel_target CLANG_FLAGS INPUT_CL TMP_DIR OUTPUT_DIR
         OUTPUT_RTL)
  set(RTL_FILE ${RTL_TARGET_NAME}${OCL_OUTPUT_EXTENSION})
  set(OUTPUT_FILE ${OUTPUT_DIR}/${RTL_FILE})
  get_filename_component(FILENAME ${INPUT_CL} NAME)
  set(OUTPUT_BC_FILE ${FILENAME}.bc)

  # generate frontend IR and run optimizer on it.
  add_custom_command(
    OUTPUT ${OUTPUT_FILE}
    DEPENDS ${CLANG} ${OPT} ${INPUT_CL} process-intrin-headers
    COMMAND ${CLANG} ${CLANG_FLAGS} ${INPUT_CL} -o ${TMP_DIR}/${OUTPUT_BC_FILE}
    COMMAND
      ${OPT} ${TMP_DIR}/${OUTPUT_BC_FILE} -verify -dpcpp-kernel-equalizer
      -dpcpp-kernel-analysis -dpcpp-kernel-wg-loop-bound
      -dpcpp-kernel-deduce-max-dim -dpcpp-kernel-wgloop-creator -loop-idiom
      -loop-deletion -dpcpp-kernel-add-implicit-args
      -dpcpp-kernel-resolve-wi-call -dpcpp-kernel-prepare-args -inline -dce
      -globaldce -instcombine -dpcpp-kernel-cleanup-wrapped -o ${OUTPUT_FILE}
    COMMENT "Compiling ${RTL_FILE}")

  add_custom_target(
    ${RTL_TARGET_NAME} ALL
    DEPENDS ${OUTPUT_FILE}
    VERBATIM
    SOURCES ${INPUT_CL})

  add_dependencies(${RTL_TARGET_NAME} ${CLANG} ${OPT})

  set_target_properties(
    ${RTL_TARGET_NAME} PROPERTIES FOLDER
                                  ${BACKEND_FOLDER_NAME}/cl_library_kernel)

  install_to(${OUTPUT_FILE} DESTINATION lib/${OUTPUT_ARCH_SUFF} COMPONENT
             ocl-library-kernel)

  set(${OUTPUT_RTL}
      ${OUTPUT_FILE}
      PARENT_SCOPE)
endfunction()

set(SOURCE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/mem_copy_set.cl)
set(CLANG_CL_COMMON_COMPILER_FLAGS ${CLANG_CL_COMMON_FLAGS} -cl-kernel-arg-info)

create_ocl_kernel_target(
  "${CLANG_CL_COMMON_COMPILER_FLAGS}" ${SOURCE_FILE} ${INTERMEDIATE_OUTPUT_DIR}
  ${LIBRARY_OUTPUT_DIR} OPTIMIZED_RTL)

set(KERNEL_RTL
    ${OPTIMIZED_RTL}
    PARENT_SCOPE)
