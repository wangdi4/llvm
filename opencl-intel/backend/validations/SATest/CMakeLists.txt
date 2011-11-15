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

if (MIC_ENABLE)
#set( COI_DIR                /opt/intel/mic/coi)
set( COI_DIR                ${BACKEND_ROOT_DIR}/../externals/coi)
set( BACKEND_SDK_PATH_RELATIVE ../devices/mic_device/BackEnd_SDK )
set( BACKEND_SDK_PATH       ${BACKEND_ROOT_DIR}/${BACKEND_SDK_PATH_RELATIVE} )
endif (MIC_ENABLE)

endif (NOT WIN32)

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
    ${LLVM_ROOT}/lib/ExecutionEngine/InterpreterPluggable
    ${LLVM_ROOT}/lib/ExecutionEngine/PlugInNEAT
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
    OpenCLReferenceRunner.cpp
    OpenCLRunConfiguration.cpp
    OpenCLProgramConfiguration.cpp
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
    OpenCLMICArgsBuffer.h
    DeviceCommunicationService.h
    )

set (SATest_SOURCE_FILES
    ${SATest_SOURCE_FILES}
    OpenCLMICBackendRunner.cpp
    OpenCLMICBackendWrapper.cpp
    OpenCLMICFactory.cpp
    OpenCLMICArgsBuffer.cpp
    DeviceCommunicationService.cpp
    )

add_definitions(-DMIC_ENABLE)

set( SATEST_NATIVE_NAME     SATestNative.bin)
list (APPEND SATestMICDefs "SATEST_NATIVE_NAME=\"${SATEST_NATIVE_NAME}\"")
list (APPEND SATestMICDefs "SVML_LIBRARY_NAME=__ocl_svml_knf.so")
list (APPEND SATestMICDefs "SVML_LIBRARY_PATH=NULL") # SATest will look SVML library in paths defined in SINK_LD_LIBRARY_PATH.
include_directories(${COI_DIR}/include)
link_directories(${COI_DIR}/host-linux-release/lib)

else(MIC_ENABLE)

set (SATest_INCLUDE_FILES
    ${SATest_INCLUDE_FILES}
    OpenCLArgsBuffer.h
    OpenCLProgramRunner.h
    OpenCLFactory.h
    )

set (SATest_SOURCE_FILES
    ${SATest_SOURCE_FILES}
    OpenCLArgsBuffer.cpp
    OpenCLProgramRunner.cpp
    OpenCLFactory.cpp
    )


endif(MIC_ENABLE)

add_executable(SATest
    ${SATest_INCLUDE_FILES}
    ${SATest_SOURCE_FILES}
)

if(MIC_ENABLE)
set_target_properties(SATest PROPERTIES COMPILE_DEFINITIONS "${SATestMICDefs}")
target_link_libraries(SATest coi_host)
endif(MIC_ENABLE)

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

if(MIC_ENABLE)
if (NOT WIN32)
add_subdirectory(MICNative)
endif(NOT WIN32)
endif(MIC_ENABLE)

install(TARGETS SATest RUNTIME DESTINATION bin)
