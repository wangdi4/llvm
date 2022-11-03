#
# INTEL CONFIDENTIAL
#
# Copyright (C) 2021-2022 Intel Corporation
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

set(TARGET_NAME OclCpuBackEnd${BUILD_PLATFORM})
set(TARGET_NAME_EMU ${TARGET_NAME}_${OUTPUT_EMU_SUFF})

project(${TARGET_NAME})
add_subdirectory(emutls)

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
  # 32 bit
  set(PLATFORM 32)
else(CMAKE_SIZEOF_VOID_P EQUAL 8)
  # 64 bit
  set(PLATFORM 64)
endif(CMAKE_SIZEOF_VOID_P EQUAL 4)

set(OCL_CPU_BACKEND_DIR ${CMAKE_CURRENT_SOURCE_DIR})

#
# Settings for Linux compilation
#
if(NOT WIN32)
  # Compiler switches that CANNOT be modified during makefile generation
  set(ADD_COMMON_C_FLAGS "-msse3 -mssse3 -fPIC -fdiagnostics-show-option")

  set(ADD_C_FLAGS "${ADD_COMMON_C_FLAGS} -std=gnu99")
  set(ADD_CXX_FLAGS "${ADD_COMMON_C_FLAGS}")

  set(ADD_C_FLAGS_DEBUG "-O0 -ggdb -D _DEBUG")
  set(ADD_C_FLAGS_RELEASE "-O2")

  # Compiler switches that CAN be modified during makefile generation and
  # configuration-independent

  # Linker switches
  set(ADD_LINKER_FLAGS_DEBUG)
  set(ADD_LINKER_FLAGS_RELEASE)

  find_package(Threads)
  set(INIT_LINKER_FLAGS
      "${INIT_LINKER_FLAGS} ${CMAKE_THREAD_LIBS_INIT} -l${CMAKE_DL_LIBS} ${RT_LIB}"
  )

  # C switches
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}         ${ADD_C_FLAGS}")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}   ${ADD_C_FLAGS_DEBUG}")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${ADD_C_FLAGS_RELEASE}")

  # C++ switches
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}         ${ADD_CXX_FLAGS}")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}   ${ADD_C_FLAGS_DEBUG}")
  set(CMAKE_CXX_FLAGS_RELEASE
      "${CMAKE_CXX_FLAGS_RELEASE} ${ADD_C_FLAGS_RELEASE}")

  # Linker switches - EXE
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${INIT_LINKER_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS_DEBUG
      "${CMAKE_EXE_LINKER_FLAGS_DEBUG}   ${ADD_LINKER_FLAGS_DEBUG}")
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE
      "${CMAKE_EXE_LINKER_FLAGS_RELEASE} ${ADD_LINKER_FLAGS_RELEASE}")

  # Linker switches - Shared Lib
  set(CMAKE_SHARED_LINKER_FLAGS
      "${CMAKE_SHARED_LINKER_FLAGS} ${INIT_LINKER_FLAGS}")
  set(CMAKE_SHARED_LINKER_FLAGS_DEBUG
      "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}   ${ADD_LINKER_FLAGS_DEBUG}")
  set(CMAKE_SHARED_LINKER_FLAGS_RELEASE
      "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} ${ADD_LINKER_FLAGS_RELEASE}")

endif(NOT WIN32)

use_rtti(FALSE)

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
    ocl_mutex.h
    ${OCL_CPU_BACKEND_DIR}/export/plugin_interface.h
    ${BACKEND_ROOT_DIR}/ocl_cpu_debugging/export/icldebuggingservice.h
    ${OCL_SOURCE_DIR}/externals/itt/include/jitprofiling.h)

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
    debuggingservicewrapper.cpp
    dllmain.cpp
    ocl_debug_builtins.cpp
    ocl_mutex.cpp
    opencl20_ext_execution.cpp
    opencl_cpu_printf_ext.cpp
    opencl_printf_ext.cpp
    opencl_task_sequence.cpp)
if(WIN32)
  list(APPEND OCL_CPU_BACKEND_INCLUDE_FILES LLDJITBuilder.h LLDJIT.h
       AsmCompiler.h)
  list(APPEND OCL_CPU_BACKEND_SOURCE_FILES LLDJITBuilder.cpp LLDJIT.cpp
       AsmCompiler.cpp ihc_thread_win32.cpp)
else(WIN32)
  list(APPEND OCL_CPU_BACKEND_SOURCE_FILES ihc_thread_linux.cpp)
endif(WIN32)

# Add include directories
set(INCLUDE_DIRS
    ${LLVM_INCLUDE_DIRS}
    ${CL_API_HEADERS}
    ${OCL_CPU_BACKEND_DIR}
    ${OCL_SOURCE_DIR}/utils/cache_binary_handler
    ${OCL_SOURCE_DIR}/utils/cl_sys_utils/export
    ${OCL_SOURCE_DIR}/utils/CLElfLib
    ${BACKEND_ROOT_DIR}/ocl_cpu_debugging/export
    ${BACKEND_ROOT_DIR}/arch_headers
    ${BACKEND_ROOT_DIR}/vectorizer/Utils
    ${BACKEND_ROOT_DIR}/optimizer
    ${BACKEND_ROOT_DIR}/plugin_manager
    ${BACKEND_ROOT_DIR}/dynamic_lib
    ${BACKEND_ROOT_DIR}/metadata_api
    ${BACKEND_ROOT_DIR}/utils/objdump
    ${CMAKE_BINARY_DIR}/include/)

# Add additional link directories
if(LLVM_USE_INTEL_JITEVENTS)
  link_directories(${LLVM_INTEL_JITEVENTS_LIBDIR})
endif(LLVM_USE_INTEL_JITEVENTS)

if(LLVM_USE_OPROFILE)
  link_directories(${LLVM_OPROFILE_INCDIR})
endif(LLVM_USE_OPROFILE)

if(WIN32 AND NOT OPENCL_INTREE_BUILD)
  # Libraries in LLD_LIBS are in LLVM_LIBRARY_DIRS directory
  link_directories(${LLVM_LIBRARY_DIRS})
endif(WIN32 AND NOT OPENCL_INTREE_BUILD)

set(BACKEND_PASSES
    LLVMBarrier
    LLVMVectorizer
    BuiltinLibInfo
    DebugInfoPass
    PrintIRPass
    SmartGVN
    StripIntelIP)

set(UTILS_LIBS # Utils lib
    OclPluginManager OclOptimizer)

set(LINK_LIBS
    LoopUtils
    OclBackendUtils
    reflection_module
    dynamic_load
    cl_sys_utils
    CacheBinaryHandler
    CLElfLib
    emutls
    soft_math
    ObjDump
    ${LLVM_MODULE_LIBS}
    LLVMIntelJITEvents
    LLVMSPIRVLib
    LLVMSymbolize)

if(WIN32)
  set(LLD_LIBS lldCOFF lldCommon)
endif(WIN32)

# Create DLL
add_opencl_library(
  ${TARGET_NAME}
  SHARED
  ${OCL_CPU_BACKEND_INCLUDE_FILES}
  ${OCL_CPU_BACKEND_SOURCE_FILES}
  ${ASM_ADD_TO_SOURCES_LIST}
  INCLUDE_DIRS
  ${INCLUDE_DIRS}
  LINK_LIBS
  ${UTILS_LIBS}
  # internal passes
  ${BACKEND_PASSES}
  ${LINK_LIBS}
  ${LLD_LIBS}
  RC_TEMPLATE
  DEFAULT
  INSTALL_PATH
  ${OUTPUT_ARCH_SUFF})

add_opencl_library(
  ${TARGET_NAME_EMU}
  SHARED
  ${OCL_CPU_BACKEND_INCLUDE_FILES}
  ${OCL_CPU_BACKEND_SOURCE_FILES}
  ${ASM_ADD_TO_SOURCES_LIST}
  INCLUDE_DIRS
  ${INCLUDE_DIRS}
  LINK_LIBS
  ${UTILS_LIBS}
  # internal passes
  ${BACKEND_PASSES}
  ${LINK_LIBS}
  ${LLD_LIBS}
  RC_TEMPLATE
  DEFAULT
  INSTALL_PATH
  ${OUTPUT_ARCH_SUFF})

set_linux_exports_file(${TARGET_NAME} ${TARGET_NAME}_linux_exports.txt)
set_linux_exports_file(${TARGET_NAME_EMU} ${TARGET_NAME}_linux_exports.txt)

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
