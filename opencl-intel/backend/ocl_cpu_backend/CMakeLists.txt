#
# INTEL CONFIDENTIAL
#
# Copyright (C) 2021 Intel Corporation
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

set(TARGET_NAME OclCpuBackEnd)

add_subdirectory(emutls)

if(BUILD_X64)
  set(PLATFORM 64)
else()
  set(PLATFORM 32)
endif()

set(OCL_CPU_BACKEND_DIR ${CMAKE_CURRENT_SOURCE_DIR})

use_rtti(FALSE)

# This macro is to provide a default path for ocl backend to find shared builtin
# libraries.
add_definitions(-DDEFAULT_OCL_LIBRARY_DIR="${OCL_OUTPUT_LIBRARY_DIR}")

# Create target project
set(OCL_CPU_BACKEND_INCLUDE_FILES
    # Interface
    IAbstractBackendFactory.h
    IDynamicFunctionsResolver.h
    IBlockToKernelMapper.h
    ICompilerConfig.h
    BackendConfiguration.h
    BitCodeContainer.h
    BlockLiteral.h
    BuiltinModules.h
    BuiltinModuleManager.h
    CPUBlockToKernelMapper.h
    CPUBuiltinLibrary.h
    CPUCompileService.h
    CPUCompiler.h
    CPUDeviceBackendFactory.h
    CPUExecutionService.h
    CPUJITContainer.h
    CPUKernel.h
    CPUProgram.h
    CPUProgramBuilder.h
    CPUSerializationService.h
    ObjectCodeCache.h
    ObjectCodeContainer.h
    CompileService.h
    Compiler.h
    CompilerConfig.h
    ExecutionService.h
    FPGAEmuBuiltinLibrary.h
    ImageCallbackLibrary.h
    ImageCallbackManager.h
    ImageCallbackServices.h
    Kernel.h
    KernelProperties.h
    LibraryProgramManager.h
    Program.h
    ProgramBuilder.h
    RuntimeService.h
    Serializer.h
    ServiceFactory.h
    StaticObjectLoader.h
    debuggingservicewrapper.h
    opencl_printf_ext.h
    ${OCL_CPU_BACKEND_DIR}/export/plugin_interface.h
    ${BACKEND_ROOT_DIR}/ocl_cpu_debugging/export/icldebuggingservice.h
    AsmCompiler.h)

set(OCL_CPU_BACKEND_SOURCE_FILES
    BackendConfiguration.cpp
    BitCodeContainer.cpp
    BuiltinModules.cpp
    BuiltinModuleManager.cpp
    CPUBlockToKernelMapper.cpp
    CPUBuiltinLibrary.cpp
    CPUCompileService.cpp
    CPUCompiler.cpp
    CPUDeviceBackendFactory.cpp
    CPUExecutionService.cpp
    CPUJITContainer.cpp
    CPUKernel.cpp
    CPUProgram.cpp
    CPUProgramBuilder.cpp
    CompileService.cpp
    Compiler.cpp
    CompilerConfig.cpp
    ExecutionService.cpp
    FPGAEmuBuiltinLibrary.cpp
    ImageCallbackLibrary.cpp
    ImageCallbackManager.cpp
    ImageCallbackServices.cpp
    Kernel.cpp
    KernelProperties.cpp
    LibraryProgramManager.cpp
    Program.cpp
    Serializer.cpp
    ObjectCodeCache.cpp
    CPUSerializationService.cpp
    ObjectCodeContainer.cpp
    ProgramBuilder.cpp
    ServiceFactory.cpp
    cpu_builtin_functions.cpp
    opencl_sort_impl.cpp
    device_barrier_impl.cpp
    debuggingservicewrapper.cpp
    dllmain.cpp
    ocl_debug_builtins.cpp
    opencl20_ext_execution.cpp
    opencl_cpu_assert_ext.cpp
    opencl_cpu_printf_ext.cpp
    opencl_printf_ext.cpp
    Intel_opencl_task_sequence.cpp  # INTEL
    AsmCompiler.cpp)
if(WIN32)
  list(APPEND OCL_CPU_BACKEND_INCLUDE_FILES LLDJITBuilder.h LLDJIT.h)
  list(APPEND OCL_CPU_BACKEND_SOURCE_FILES LLDJITBuilder.cpp LLDJIT.cpp
       ihc_thread_win32.cpp)
else(WIN32)
  list(APPEND OCL_CPU_BACKEND_SOURCE_FILES ihc_thread_linux.cpp)
endif(WIN32)

# Add include directories
set(INCLUDE_DIRS
    ${LLVM_INCLUDE_DIRS}
    ${LLVM_EXTERNAL_LLVM_SPIRV_SOURCE_DIR}/lib
    ${CL_API_HEADERS}
    ${OCL_CPU_BACKEND_DIR}
    ${OCL_SOURCE_DIR}/utils/cache_binary_handler
    ${OCL_SOURCE_DIR}/utils/cl_sys_utils/export
    ${OCL_SOURCE_DIR}/utils/CLElfLib
    ${BACKEND_ROOT_DIR}/ocl_cpu_debugging/export
    ${BACKEND_ROOT_DIR}/arch_headers
    ${BACKEND_ROOT_DIR}/optimizer
    ${BACKEND_ROOT_DIR}/plugin_manager
    ${BACKEND_ROOT_DIR}/dynamic_lib
    ${BACKEND_ROOT_DIR}/metadata_api
    ${BACKEND_ROOT_DIR}/utils/objdump
    ${CMAKE_BINARY_DIR}/include)

if(WIN32)
  list(APPEND INCLUDE_DIRS ${LLVM_MAIN_SRC_DIR}/../lld/include)
endif(WIN32)

# Create ocl backend lib
add_opencl_library(
  ${TARGET_NAME}
  STATIC
  ${OCL_CPU_BACKEND_INCLUDE_FILES}
  ${OCL_CPU_BACKEND_SOURCE_FILES}
  ${ASM_ADD_TO_SOURCES_LIST}
  INCLUDE_DIRS
  ${INCLUDE_DIRS}
  COMPONENTS
  CacheBinaryHandler
  ObjDump
  OclBackendUtils
  OclOptimizer
  cl_sys_utils
  emutls
  soft_math
  task_executor
  LINK_LIBS
  ${LLVM_MODULE_LIBS})

add_dependencies(${TARGET_NAME} intrinsics_gen)

set(CMAKE_CXX_FLAGS_DEBUG "-DOCL_DEV_BACKEND_PLUGINS ${CMAKE_CXX_FLAGS_DEBUG}")

add_definitions(-DOclCpuBackEnd_EXPORTS)
add_definitions(-DOCL_OUTPUT_EXTENSION="${OCL_OUTPUT_EXTENSION}")
add_definitions(
  -DOCL_PRECOMPILED_OUTPUT_EXTENSION="${OCL_PRECOMPILED_OUTPUT_EXTENSION}")
add_definitions(-DOCL_LIBRARY_TARGET_NAME="${OCL_LIBRARY_TARGET_NAME}")

if(ENABLE_SDE)
  message(STATUS "-- Enabling SDE version ... ")
  add_definitions(-DENABLE_SDE)
endif(ENABLE_SDE)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME})
set_target_properties(${TARGET_NAME_EMU} PROPERTIES FOLDER
                                                    ${BACKEND_FOLDER_NAME})

link_target_with_tbb_library(${TARGET_NAME})
