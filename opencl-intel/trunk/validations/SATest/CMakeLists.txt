#if(NOT WIN32)
#add_definitions(-msse4.2)
#endif(NOT WIN32)

if(WIN32)
    set (OS_PLATFORM Win32)
else(WIN32)
    set (OS_PLATFORM Linux64)
endif(WIN32)

if( CMAKE_SIZEOF_VOID_P EQUAL 4)
    # 32 bit
    set (PLATFORM 32)
else( CMAKE_SIZEOF_VOID_P EQUAL 4)
    # 64 bit
    set (PLATFORM 64)
endif( CMAKE_SIZEOF_VOID_P EQUAL 4)

#
# Settings for Linux compilation
#
if (NOT WIN32)

# Compiler switches that CANNOT be modified during makefile generation
set (ADD_COMMON_C_FLAGS  "-msse4.2")

set (ADD_C_FLAGS         "${ADD_COMMON_C_FLAGS} ")
set (ADD_CXX_FLAGS       "${ADD_COMMON_C_FLAGS} ")

set (ADD_C_FLAGS_DEBUG   "")
set (ADD_C_FLAGS_RELEASE "")

# Compiler switches that CAN be modified during makefile generation and configuration-independent
add_definitions( ${WARNING_LEVEL} )

# Linker switches
set (INIT_LINKER_FLAGS        ) 
set (ADD_LINKER_FLAGS_DEBUG   )
set (ADD_LINKER_FLAGS_RELEASE )

include(FindThreads)
set( INIT_LINKER_FLAGS "${INIT_LINKER_FLAGS} ${CMAKE_THREAD_LIBS_INIT} -l${CMAKE_DL_LIBS} -lrt" ) 

# C switches
set( CMAKE_C_FLAGS         "${CMAKE_C_FLAGS}         ${ADD_C_FLAGS}")
set( CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG}   ${ADD_C_FLAGS_DEBUG}")
set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${ADD_C_FLAGS_RELEASE}")

# C++ switches
set( CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS}         ${ADD_CXX_FLAGS}")
set( CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   ${ADD_C_FLAGS_DEBUG}")
set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${ADD_C_FLAGS_RELEASE}")

# Linker switches - EXE
set( CMAKE_EXE_LINKER_FLAGS           ${INIT_LINKER_FLAGS})
set( CMAKE_EXE_LINKER_FLAGS_DEBUG     "${CMAKE_EXE_LINKER_FLAGS_DEBUG}   ${ADD_LINKER_FLAGS_DEBUG}")
set( CMAKE_EXE_LINKER_FLAGS_RELEASE   "${CMAKE_EXE_LINKER_FLAGS_RELEASE} ${ADD_LINKER_FLAGS_RELEASE}")

# Linker switches - Shared Lib
set( CMAKE_SHARED_LINKER_FLAGS          ${INIT_LINKER_FLAGS})
set( CMAKE_SHARED_LINKER_FLAGS_DEBUG   "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}   ${ADD_LINKER_FLAGS_DEBUG}")
set( CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} ${ADD_LINKER_FLAGS_RELEASE}")

endif (NOT WIN32)

include_directories(
    ${CMAKE_SOURCE_DIR}/../validations/SATest
    ${CMAKE_SOURCE_DIR}/../validations/DataManager
    ${CMAKE_SOURCE_DIR}/../validations/NEAT
    ${CMAKE_SOURCE_DIR}/../validations/Comparator
    ${CMAKE_SOURCE_DIR}/../validations/Common
    ${CMAKE_SOURCE_DIR}/../validations/OCLBuiltins
    ${CMAKE_SOURCE_DIR}/../build/${OS_PLATFORM}/include
    ${CMAKE_SOURCE_DIR}/../external/tinyxml
    ${CMAKE_SOURCE_DIR}/../cl_api
    ${CMAKE_SOURCE_DIR}/../ocl_cpu_backend
    ${CMAKE_SOURCE_DIR}/../ocl_cpu_backend/export
    ${CMAKE_SOURCE_DIR}/../arch_headers
    ${CMAKE_SOURCE_DIR}/lib/ExecutionEngine/Interpreter
    ${CMAKE_SOURCE_DIR}/lib/ExecutionEngine/InterpreterPluggable
    ${CMAKE_SOURCE_DIR}/lib/ExecutionEngine/PlugInNEAT
)

set (SATest_INCLUDE_FILES
    auto_ptr_ex.h
    mem_utils.h
    IRunnerFactory.h
    IProgram.h
    IProgramRunner.h
    IProgramConfiguration.h
    IRunConfiguration.h
    IRunResult.h
    IRunResultComparator.h
    IPerformance.h
    OpenCLProgram.h
    OpenCLProgramRunner.h
    OpenCLReferenceRunner.h
    OpenCLFactory.h
    OpenCLRunConfiguration.h
    OpenCLProgramConfiguration.h
    OpenCLArgsBuffer.h
    OpenCLBackendWrapper.h
    OpenCLComparator.h
    OCLUpdatePass.h
    RunResult.h
    Performance.h
    RunnerFactoryGenerator.h
    SATest.h
    SATestException.h
    DXProgram.h
    DXProgramRunner.h
    DXreferenceRunner.h
    DXFactory.h
    DXRunConfiguration.h
    DXProgramConfiguration.h
)

set (SATest_SOURCE_FILES
    mem_utils.cpp
    dbg_utils.cpp
    main.cpp
    RunResult.cpp
    Performance.cpp
    RunnerFactoryGenerator.cpp
    SATest.cpp
    DXReferenceRunner.cpp
    DXFactory.cpp
    DXRunConfiguration.cpp
    DXProgram.cpp
    DXProgramRunner.cpp
    DXProgramConfiguration.cpp
    OpenCLProgram.cpp
    OpenCLProgramRunner.cpp
    OpenCLReferenceRunner.cpp
    OpenCLFactory.cpp
    OpenCLRunConfiguration.cpp
    OpenCLProgramConfiguration.cpp
    OpenCLArgsBuffer.cpp
    OpenCLBackendWrapper.cpp
    OpenCLComparator.cpp
    OCLUpdatePass.cpp
    )

if(MIC_ENABLE)
set (SATest_INCLUDE_FILES
    ${SATest_INCLUDE_FILES}
    OpenCLMICBackendRunner.h
    OpenCLMICBackendWrapper.h
    OpenCLMICFactory.h
    )

set (SATest_SOURCE_FILES
    ${SATest_SOURCE_FILES}
    OpenCLMICBackendRunner.cpp
    OpenCLMICBackendWrapper.cpp
    OpenCLMICFactory.cpp
    )

add_definitions(-DMIC_ENABLE)
endif(MIC_ENABLE)

add_executable(SATest
    ${SATest_INCLUDE_FILES}
    ${SATest_SOURCE_FILES}
)

 set (LLVM_LIBS
    # LLVM libs
    LLVMBitReader

    LLVMVectorizer
    LLVMipo

    LLVMLinker
    LLVMArchive

    LLVMCodeGen
    LLVMScalarOpts
    LLVMTransformUtils
    LLVMipa
    LLVMAnalysis
    LLVMBitWriter

    LLVMExecutionEngine
    LLVMTarget
    LLVMCore
    LLVMSupport
    OclUtils
    LLVMAsmParser
    )

set (SATest_DEPS
    DataManager
    Comparator
    LLVMInterpreter
    LLVMInterpreterPluggable
    ${LLVM_LIBS}
    NEAT
    PlugInNEAT
    )

target_link_libraries(SATest
    ${SATest_DEPS}
    )

add_dependencies(SATest
    ${SATest_DEPS}
    OclCpuBackEnd
    )

