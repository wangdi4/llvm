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

if (INCLUDE_MIC_DEVICE)
#set( COI_DIR                /opt/intel/mic/coi)
set( COI_DIR                ${BACKEND_ROOT_DIR}/../externals/coi)
endif (INCLUDE_MIC_DEVICE)

include_directories(
    ${VALIDATIONS_ROOT}/SATest
    ${VALIDATIONS_ROOT}/DataManager
    ${VALIDATIONS_ROOT}/NEAT
    ${VALIDATIONS_ROOT}/Comparator
    ${VALIDATIONS_ROOT}/Common
    ${VALIDATIONS_ROOT}/OCLBuiltins
    ${VALIDATIONS_ROOT}/../external/tinyxml
    ${BACKEND_ROOT_DIR}/../cl_api
    ${VALIDATIONS_ROOT}/../ocl_cpu_backend
    ${VALIDATIONS_ROOT}/../ocl_cpu_backend/export
    ${BACKEND_ROOT_DIR}/arch_headers
    ${LLVM_ROOT}/lib/ExecutionEngine/Interpreter
    ${VALIDATIONS_ROOT}/InterpreterPluggable
    ${VALIDATIONS_ROOT}/PlugInNEAT
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
    OpenCLReferenceRunner.h
    OpenCLRunConfiguration.h
    OpenCLProgramConfiguration.h
    OpenCLBackendWrapper.h
    OpenCLComparator.h
    RunResult.h
    Performance.h
    PerformancePrinter.h
    RunnerFactoryGenerator.h
    SATest.h
    SATestException.h
    DXProgram.h
    DXProgramRunner.h
    DXreferenceRunner.h
    DXFactory.h
    DXRunConfiguration.h
    DXProgramConfiguration.h
    WorkItemStorage.h
    OpenCLArgsBuffer.h
    OpenCLCPUBackendRunner.h
    OpenCLFactory.h
    BackendOptions.h
    OpenCLBackendRunner.h
)

set (SATest_SOURCE_FILES
    mem_utils.cpp
    dbg_utils.cpp
    main.cpp
    RunResult.cpp
    Performance.cpp
    PerformancePrinter.cpp
    RunnerFactoryGenerator.cpp
    SATest.cpp
    DXReferenceRunner.cpp
    DXFactory.cpp
    DXRunConfiguration.cpp
    DXProgram.cpp
    DXProgramRunner.cpp
    DXProgramConfiguration.cpp
    OpenCLProgram.cpp
    OpenCLReferenceRunner.cpp
    OpenCLRunConfiguration.cpp
    OpenCLProgramConfiguration.cpp
    OpenCLBackendWrapper.cpp
    OpenCLComparator.cpp
    OpenCLArgsBuffer.cpp
    OpenCLCPUBackendRunner.cpp
    OpenCLFactory.cpp
    OpenCLBackendRunner.cpp
    )

if (INCLUDE_MIC_DEVICE)
set (SATest_INCLUDE_FILES
    ${SATest_INCLUDE_FILES}
    OpenCLMICBackendRunner.h
    OpenCLMICArgsBuffer.h
    COIHelpers.h
    )
set (SATest_SOURCE_FILES
    ${SATest_SOURCE_FILES}
    OpenCLMICBackendRunner.cpp
    OpenCLMICArgsBuffer.cpp
    COIHelpers.cpp
    )
add_definitions( -DINCLUDE_MIC_TARGET )
set( SATEST_NATIVE_NAME     SATestNative.bin)
list (APPEND SATestMICDefs "SATEST_NATIVE_NAME=\"${SATEST_NATIVE_NAME}\"")
list (APPEND SATestMICDefs "SVML_LIBRARY_PATH=NULL") # SATest will look SVML library in paths defined in SINK_LD_LIBRARY_PATH.
include_directories(${COI_DIR}/include)
link_directories(${COI_DIR}/host-linux-release/lib)
set( BACKEND_SDK_PATH_RELATIVE ../devices/mic_device/BackEnd_SDK )
set( BACKEND_SDK_PATH       ${BACKEND_ROOT_DIR}/${BACKEND_SDK_PATH_RELATIVE} )
endif (INCLUDE_MIC_DEVICE)

add_executable(SATest
    ${SATest_INCLUDE_FILES}
    ${SATest_SOURCE_FILES}
)

if (INCLUDE_MIC_DEVICE)
set_target_properties(SATest PROPERTIES COMPILE_DEFINITIONS "${SATestMICDefs}")
target_link_libraries(SATest coi_host)
# Copy libcoi_host.so and libcoi_host.so.0 to the bin directory, so no need
# adding ${COI_DIR}/host-linux-release/lib to the LD_LIBRARY_PATH directory.
install(FILES ${COI_DIR}/host-linux-release/lib/libcoi_host.so DESTINATION bin)
install(FILES ${COI_DIR}/host-linux-release/lib/libcoi_host.so.0 DESTINATION bin)
endif (INCLUDE_MIC_DEVICE)

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

if (INCLUDE_MIC_DEVICE)
add_subdirectory(MICNative)
add_dependencies(SATest
    OclDeviceExecutor
    )
endif (INCLUDE_MIC_DEVICE)

install(TARGETS SATest RUNTIME DESTINATION bin)
