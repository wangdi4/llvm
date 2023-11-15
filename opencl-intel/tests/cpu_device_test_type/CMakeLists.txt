set(TARGET cpu_device_test_type)

include_directories(AFTER ${OCL_SOURCE_DIR}/utils/cl_sys_utils/export)

find_sources(${IGNORE_FILES})
calculate_target_sources()

set(CPU_DEVICE_TEST_TYPE_LINK_LIBS
  ${LLVM_MODULE_LIBS}
  BuiltinLibInfo
  CLElfLib
  CacheBinaryHandler
  LLVMBinaryFormat
  LLVMDebugInfoBTF
  LLVMDebugInfoDWARF
  LLVMDebugInfoTraceBack
  LLVMInstrumentation
  LLVMIntelJITEvents
  LLVMIntel_Debug
  LLVMMC
  LLVMMCDisassembler
  LLVMMCParser
  LLVMObject
  LLVMOrcTargetProcess
  LLVMSPIRVLib
  LLVMSymbolize
  LLVMTargetParser
  LLVMVPOTransforms
  LLVMVectorize
  ObjDump
  OclBackendUtils
  OclCpuBackEnd
  OclOptimizer
  OclPluginManager
  builtin_kernels
  cl_logger
  cl_hw_utils
  cl_sys_utils
  cpu_device
  device_commands
  dynamic_load
  emutls
  reflection_module
  soft_math
  task_executor
  test_utils)

if(WIN32)
  list(APPEND CPU_DEVICE_TEST_TYPE_LINK_LIBS delayimp.lib lldCOFF lldCommon)
endif()

set(INPUT_FILE test.cl)
set(OUTPUT_FILE test.bc)
compile_bitcode(${INPUT_FILE} ${OUTPUT_FILE})

set(TEST_BC_FILES ${TARGET_NAME}_test_files)
add_custom_target(${TEST_BC_FILES} DEPENDS ${OUTPUT_FILE})

add_ocl_unittest(${TARGET} SOURCE_FILES ${TARGET_SOURCES} LINK_LIBRARIES
                 ${CPU_DEVICE_TEST_TYPE_LINK_LIBS})

add_dependencies(${TARGET} clang_compiler ${OCL_RT_TEST_DEPS} ${TEST_BC_FILES})

link_target_with_tbb_library(${TARGET})

