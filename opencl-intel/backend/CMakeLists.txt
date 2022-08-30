# Copyright (C) 2011-2022 Intel Corporation
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

if (NOT OPENCL_INTREE_BUILD)
  # LLVM
  if( NOT DEFINED LLVM_PATH_BE )
    message( FATAL_ERROR "LLVM_PATH_BE is not specified. Please specify LLVM library location for backend using LLVM_PATH_BE parameter to CMAKE" )
  endif()

  set(LLVM_PATH ${LLVM_PATH_BE})
  find_package(LLVM REQUIRED)
  set(CLANG_BIN_PATH "${LLVM_BINARY_DIR}/clang")
else()
  set(CLANG_BIN_PATH "${LLVM_BINARY_DIR}/bin/clang")
endif()

set( INTEL_COLLAB ON )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DINTEL_COLLAB=1" )

set( BACKEND_BUILD_TESTS            ON  CACHE STRING "")
set( BACKEND_BUILD_VERIFICATION_LIB ON  CACHE STRING "")

set( OCL_BACKEND_STAND_ALONE      ON  CACHE STRING "")

set_property( GLOBAL PROPERTY USE_FOLDERS ON )
set_property( GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "__BUILD__")

set( BACKEND_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set( BACKEND_FOLDER_NAME       Backend )
set( BACKEND_TESTS_FOLDER_NAME  ${BACKEND_FOLDER_NAME}/tests )
set( BACKEND_PASSES_FOLDER_NAME  ${BACKEND_FOLDER_NAME}/passes )
set( BACKEND_VALIDATION_FOLDER_NAME  ${BACKEND_FOLDER_NAME}/validation )
if (NOT OPENCL_INTREE_BUILD)
  set_directory_properties(PROPERTIES INCLUDE_DIRECTORIES "")
endif()

# Version of cached binary.
# This value should be incremented each time cached binary format changes.
add_definitions(-DOCL_CACHED_BINARY_VERSION=16)

set(OCL_OUTPUT_EXTENSION .rtl)
set(OCL_PRECOMPILED_OUTPUT_EXTENSION .o)
set(OCL_LIBRARY_TARGET_NAME cllibrary)

file(GLOB_RECURSE OCL_ALL_TD_FILES "${BACKEND_ROOT_DIR}/libraries/ocl_builtins/*.td")

SET_UNICODE_OFF()

#
# Settings for Linux compilation
#
if (NOT WIN32 AND (CMAKE_BUILD_TYPE MATCHES "Release"))
    add_definitions(-D_FORTIFY_SOURCE=2)
endif()

###############################################################################################

project(Backend C CXX)

add_definitions(${LLVM_DEFINITIONS})
use_rtti(FALSE)

include_directories (BEFORE
                     ${BACKEND_ROOT_DIR}/utils
                     ${OCL_SOURCE_DIR}/cl_api)

if (OPENCL_INTREE_BUILD)
  llvm_map_components_to_libnames(LLVM_MODULE_LIBS
    AllTargetsAsmParsers
    AllTargetsCodeGens
    AllTargetsDescs
    AllTargetsDisassemblers
    AllTargetsInfos
    Analysis
    BitWriter
    CodeGen
    Core
    ExecutionEngine
    MCJIT
    IPO
    IRReader
    InstCombine
    Linker
    Option
    OrcJIT
    Passes
    ScalarOpts
    Support
    Target
    TransformUtils
    Vectorize
    Intel_DPCPPKernelTransforms
    Intel_OpenCLTransforms
  )
endif()

if (LLVM_BUILD_OCL_CPU_BACKEND)
    add_subdirectory(barrier                "${CMAKE_CURRENT_BINARY_DIR}/barrier")
    add_subdirectory(dynamic_lib            "${CMAKE_CURRENT_BINARY_DIR}/dynamic_lib")
    add_subdirectory(libraries              "${CMAKE_CURRENT_BINARY_DIR}/libraries")
    add_subdirectory(ocl_cpu_backend        "${CMAKE_CURRENT_BINARY_DIR}/ocl_cpu_backend")
    add_subdirectory(ocl_cpu_debugging      "${CMAKE_CURRENT_BINARY_DIR}/ocl_cpu_debugging")
    add_subdirectory(ocl_opt                "${CMAKE_CURRENT_BINARY_DIR}/ocl_opt")
    add_subdirectory(optimizer              "${CMAKE_CURRENT_BINARY_DIR}/optimizer")
    add_subdirectory(passes                 "${CMAKE_CURRENT_BINARY_DIR}/passes")
    add_subdirectory(plugin_manager         "${CMAKE_CURRENT_BINARY_DIR}/plugin_manager")
    add_subdirectory(reflection_module      "${CMAKE_CURRENT_BINARY_DIR}/reflection_module")
    add_subdirectory(stat_summary           "${CMAKE_CURRENT_BINARY_DIR}/stat_summary")
    add_subdirectory(utils                  "${CMAKE_CURRENT_BINARY_DIR}/utils")
    add_subdirectory(vectorizer             "${CMAKE_CURRENT_BINARY_DIR}/vectorizer")
endif (LLVM_BUILD_OCL_CPU_BACKEND)

if (BACKEND_BUILD_VERIFICATION_LIB)
    add_subdirectory(external/tinyxml  "${CMAKE_CURRENT_BINARY_DIR}/tinyxml")
    add_subdirectory(validations       "${CMAKE_CURRENT_BINARY_DIR}/validations")

    set_target_properties( DataVersion PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation )
    set_target_properties( Comparator PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation )
    set_target_properties( DataGenerator PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation )
    set_target_properties( DataManager PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation )
    if (NOT WIN32)
        set_target_properties( NEAT PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation )
        set_target_properties( REFALU PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation )
    endif ()
    set_target_properties( PlugInNEAT PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation )
    set_target_properties( OCLBuiltinsRef PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation )
    set_target_properties( SATest PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation )
    set_target_properties( LLVMInterpreterPluggable PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation )
    set_target_properties( ValidationTests PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation/tests )
    set_target_properties( NEATChecker PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation/tests )
    set_target_properties( FMAtest PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation/tests )
    set_target_properties( tinyxml_STL PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/externals )
endif (BACKEND_BUILD_VERIFICATION_LIB)

if (OPENCL_BE_INCLUDE_TESTS)
    add_subdirectory(tests)
endif (OPENCL_BE_INCLUDE_TESTS)
