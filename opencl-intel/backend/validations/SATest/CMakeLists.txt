#
# Settings for Linux compilation
#
set(TARGET_NAME SATest)
use_rtti(FALSE)

if (NOT WIN32)
    if (INCLUDE_MIC_DEVICE)
        set (SATEST_INCLUDE_MIC_DEVICE INCLUDE_MIC_DEVICE)
    endif (INCLUDE_MIC_DEVICE)
endif (NOT WIN32)

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

if (SATEST_INCLUDE_MIC_DEVICE)
    if (KNC_CARD)
        if ( NOT WIN32)
            add_library(../../external/vtune/libknc/jitprofiling.a STATIC IMPORTED)
        endif(NOT WIN32)
    endif (KNC_CARD)

	add_definitions ( -DCOI_LIBRARY_VERSION=2 )

endif (SATEST_INCLUDE_MIC_DEVICE)

if (WIN32)
    set (ICC_WIN32_PATH $ENV{ICPP_COMPILER12})

    if (CMAKE_SIZEOF_VOID_P EQUAL 4)
      set (ICC_PLATFORM ia32)
    else ()
      set (ICC_PLATFORM intel64)
    endif ()

    link_directories( "${ICC_WIN32_PATH}/compiler/lib/${ICC_PLATFORM}" )
endif(WIN32)


link_directories(
    ${LLVM_LIBRARY_DIR}
)

if( NOT WIN32 )
    link_directories(${OCL_BINARY_DIR}/lib/${CMAKE_CFG_INTDIR})
endif( NOT WIN32)


include_directories(
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

if (SATEST_INCLUDE_MIC_DEVICE)
    set (TARGET_HEADER_FILES
        ${TARGET_HEADER_FILES}
        OpenCLMICBackendRunner.h
        OpenCLMICArgsBuffer.h
        COIHelpers.h
        )
    set (TARGET_SOURCE_FILES
        ${TARGET_SOURCE_FILES}
        OpenCLMICBackendRunner.cpp
        OpenCLMICArgsBuffer.cpp
        COIHelpers.cpp
        )
    add_definitions( -DSATEST_INCLUDE_MIC_DEVICE )
    set( SATEST_NATIVE_NAME     SATestNative.bin)
    list (APPEND SATestMICDefs "SATEST_NATIVE_NAME=\"${SATEST_NATIVE_NAME}\"")
    list (APPEND SATestMICDefs "SVML_LIBRARY_PATH=NULL") # SATest will look SVML library in paths defined in SINK_LD_LIBRARY_PATH.
    include_directories(${COI_HOST_INCLUDE_PATH} ${COI_YOCTO_HOST_INCLUDE_PATH})
    link_directories(${COI_HOST_LIB_PATH} ${COI_YOCTO_HOST_LIB_PATH})
    set( BACKEND_SDK_PATH_RELATIVE ../devices/mic_device/BackEnd_SDK )
    set( BACKEND_SDK_PATH       ${BACKEND_ROOT_DIR}/${BACKEND_SDK_PATH_RELATIVE} )
endif (SATEST_INCLUDE_MIC_DEVICE)

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
    
if (SATEST_INCLUDE_MIC_DEVICE)
    set_target_properties(${TARGET_NAME} PROPERTIES COMPILE_DEFINITIONS "${SATestMICDefs}")
    target_link_libraries(${TARGET_NAME} coi_host dynamic_load cl_sys_utils)

    if (NOT WIN32)
	get_target_property( CURR_RPATH ${TARGET_NAME} INSTALL_RPATH )
	set_target_properties( ${TARGET_NAME} PROPERTIES INSTALL_RPATH "${CURR_RPATH};${COI_HOST_LIB_PATH}" )
    endif (NOT WIN32)
endif (SATEST_INCLUDE_MIC_DEVICE)

if (NOT WIN32)
  if(NOT ANDROID)
    target_link_libraries(${TARGET_NAME} pthread dl)
  endif(NOT ANDROID)
endif (NOT WIN32)

if(NOT WIN32)
    add_dependencies(${TARGET_NAME} ImathLibd_wrapper)
endif(NOT WIN32)

if (SATEST_INCLUDE_MIC_DEVICE)
    add_subdirectory(MICNative)
    add_dependencies(${TARGET_NAME}
                     OclDeviceExecutor
    )
endif (SATEST_INCLUDE_MIC_DEVICE)

install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION bin)
