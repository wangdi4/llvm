# INTEL CONFIDENTIAL
#
# Copyright 2022 Intel Corporation.
#
# This software and the related documents are Intel copyrighted materials, and
# your use of them is governed by the express license under which they were
# provided to you (License). Unless the License provides otherwise, you may not
# use, modify, copy, publish, distribute, disclose or transmit this software or
# the related documents without Intel's prior written permission.
#
# This software and the related documents are provided as is, with no express or
# implied warranties, other than those that are expressly stated in the License.

set(TARGET_NAME soft_math)

set(SOFT_MATH_SOURCE_FILES extendhfsf2.c int_util.c truncdfhf2.c truncsfhf2.c)

# Convert compile flags to list to facilitate custom command
string(REPLACE " " ";" SOFT_MATH_CFLAGS ${CMAKE_C_FLAGS})

# Ignore incompatible compiler options from CMAKE_C_FLAGS
list(APPEND SOFT_MATH_CFLAGS -Wno-unknown-argument)

if(NOT WIN32)
  set(COMPILER ${CLANG_BIN_PATH})
  set(OBJECT_SUFFIX .o)
else()
  set(COMPILER ${CLANG_CL_BIN_PATH})
  set(OBJECT_SUFFIX .obj)
endif()

if(NOT WIN32)
  list(
    APPEND
    SOFT_MATH_SOURCE_FILES
    extenddftf2.c
    extendhftf2.c
    extendsftf2.c
    trunctfdf2.c
    trunctfhf2.c
    trunctfsf2.c)
  # Enable float 128
  list(APPEND SOFT_MATH_CFLAGS -mlong-double-128)
endif(NOT WIN32)

if(WIN32 AND BUILD_X64)
  list(APPEND SOFT_MATH_SOURCE_FILES divti3.c udivti3.c udivmodti4.c)
endif()

# On win32, ocl is built with msvc, so we need to tell clang not to link the
# default libs to avoid symbol conflicts.
if(WIN32 AND NOT BUILD_X64)
  list(APPEND SOFT_MATH_CFLAGS -m32 /Zl)
endif()

# Manually compile source files with clang
foreach(FILE ${SOFT_MATH_SOURCE_FILES})
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${FILE}${OBJECT_SUFFIX}
    COMMAND
      ${COMPILER} ${SOFT_MATH_CFLAGS} -c ${CMAKE_CURRENT_SOURCE_DIR}/${FILE} -o
      ${CMAKE_CURRENT_BINARY_DIR}/${FILE}${OBJECT_SUFFIX}
    DEPENDS ${CLANG} ${CMAKE_CURRENT_SOURCE_DIR}/${FILE}
    COMMENT "Compiling ${FILE}")

  list(APPEND SOFT_MATH_OUTPUT_OBJECTS
       ${CMAKE_CURRENT_BINARY_DIR}/${FILE}${OBJECT_SUFFIX})
endforeach()

add_custom_target(soft_math_obj ALL DEPENDS ${SOFT_MATH_OUTPUT_OBJECTS})

set_source_files_properties(${SOFT_MATH_OUTPUT_OBJECTS}
                            PROPERTIES EXTERNAL_OBJECT true GENERATED true)

add_opencl_library(${TARGET_NAME} STATIC ${SOFT_MATH_OUTPUT_OBJECTS})

add_dependencies(${TARGET_NAME} soft_math_obj)

set_target_properties(
  ${TARGET_NAME}
  PROPERTIES # For a tidy Visual Studio solution layout
             FOLDER ${BACKEND_FOLDER_NAME}
             # We need to let cmake know we want a c library
             LINKER_LANGUAGE C)
