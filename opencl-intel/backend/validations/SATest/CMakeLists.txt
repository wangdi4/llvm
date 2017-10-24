if( NOT DEFINED LLVM_PATH_FE )
    message( FATAL_ERROR "LLVM_PATH_FE is not specified. Please specify LLVM library location for backend using LLVM_PATH_FE parameter to CMAKE" )
endif()
set (LLVM_FE_INCLUDE_DIRS ${LLVM_PATH_FE}/include)
#
# Settings for Linux compilation
#
set(TARGET_NAME SATest)
use_rtti(FALSE)

if (WIN32)
    set (ICC_WIN32_PATH $ENV{ICPP_COMPILER16})

    if ("${ICC_WIN32_PATH}" STREQUAL "")
      message (FATAL_ERROR "ICC Compiler env var is not exported! SATest won't link.")
    endif()

    if (CMAKE_SIZEOF_VOID_P EQUAL 4)
      set (ICC_PLATFORM ia32)
    else ()
      set (ICC_PLATFORM intel64)
    endif ()

    link_directories( "${ICC_WIN32_PATH}/compiler/lib/${ICC_PLATFORM}" )
endif(WIN32)

if( NOT WIN32 )
    link_directories(${OCL_BINARY_DIR}/lib/${CMAKE_CFG_INTDIR})
endif( NOT WIN32)

link_directories(${LLVM_LIBRARY_DIRS})

include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${LLVM_FE_INCLUDE_DIRS}/cclang
  ${VALIDATIONS_ROOT}/SATest
  ${VALIDATIONS_ROOT}/DataVersion
  ${VALIDATIONS_ROOT}/DataManager
  ${VALIDATIONS_ROOT}/NEAT
  ${VALIDATIONS_ROOT}/REFALU
  ${VALIDATIONS_ROOT}/Comparator
  ${VALIDATIONS_ROOT}/Common
  ${VALIDATIONS_ROOT}/OCLBuiltins
  ${VALIDATIONS_ROOT}/../external/tinyxml
  ${BACKEND_ROOT_DIR}/../cl_api
  ${CL_API_HEADERS}
  ${VALIDATIONS_ROOT}/../ocl_cpu_backend
  ${VALIDATIONS_ROOT}/../ocl_cpu_backend/export
  ${BACKEND_ROOT_DIR}/utils
  ${BACKEND_ROOT_DIR}/arch_headers
  ${BACKEND_ROOT_DIR}/dynamic_lib
  ${BACKEND_ROOT_DIR}/name_mangling
  ${LLVM_ROOT}/lib/ExecutionEngine/Interpreter
  ${VALIDATIONS_ROOT}/InterpreterPluggable
  ${VALIDATIONS_ROOT}/PlugInNEAT
  ${VALIDATIONS_ROOT}/OCLBuilder
  ${VALIDATIONS_ROOT}/OpenCLKernelArgumentsParser
  ${VALIDATIONS_ROOT}/OCLKernelDataGenerator
  ${CMAKE_SOURCE_DIR}/utils/cl_sys_utils/export
  ${VALIDATIONS_ROOT}/ProgramConfiguration
)

set (TARGET_HEADER_FILES
    IRunnerFactory.h
    IProgram.h
    IProgramRunner.h
    IRunConfiguration.h
    IRunResult.h
    IRunResultComparator.h
    IPerformance.h
    OpenCLProgram.h
    OpenCLReferenceRunner.h
    OpenCLRunConfiguration.h
    OpenCLBackendWrapper.h
    OpenCLComparator.h
    RunResult.h
    Performance.h
    PerformancePrinter.h
    RunnerFactoryGenerator.h
    SATest.h
    SATestException.h
    WorkItemStorage.h
    WorkGroupStorage.h
    OpenCLArgsBuffer.h
    OpenCLCPUBackendRunner.h
    OpenCLFactory.h
    BackendOptions.h
    OpenCLBackendRunner.h
    ${VALIDATIONS_ROOT}/Common/md5.h
    OpenCLStamp.h
)

set (TARGET_SOURCE_FILES
    mem_utils.cpp
    dbg_utils.cpp
    main.cpp
    RunResult.cpp
    Performance.cpp
    PerformancePrinter.cpp
    RunnerFactoryGenerator.cpp
    SATest.cpp
    OpenCLProgram.cpp
    OpenCLReferenceRunner.cpp
    OpenCLRunConfiguration.cpp
    OpenCLBackendWrapper.cpp
    OpenCLComparator.cpp
    OpenCLArgsBuffer.cpp
    OpenCLCPUBackendRunner.cpp
    OpenCLFactory.cpp
    OpenCLBackendRunner.cpp
    ${VALIDATIONS_ROOT}/Common/md5.cpp
    OpenCLStamp.cpp
    )

add_executable(${TARGET_NAME}
    ${TARGET_HEADER_FILES}
    ${TARGET_SOURCE_FILES}
)

target_link_libraries(${TARGET_NAME}
    OpenCLKernelArgumentsParser
    OCLKernelDataGenerator
    OCLBuilder
    PlugInNEAT
    DataManager
    DataVersion
    Comparator
    LLVMInterpreterPluggable
    OclBackendUtils
    LLVMInterpreter
    OCLBuiltins
    NEAT
    REFALU
    name_mangle
    ProgramConfiguration
    dynamic_load
    tinyxml_STL
    ImathLibd
    cl_sys_utils
    ${LLVM_MODULE_LIBS}
    )

#if (NOT WIN32)
#    SET_LINUX_EXPORTS_FILE( ${TARGET_NAME} OCLBuiltins_linux_exports.txt )
#endif (NOT WIN32)

if(NOT WIN32)
    add_dependencies(${TARGET_NAME} ImathLibd_wrapper)
endif(NOT WIN32)

install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION tests/LITTests)
