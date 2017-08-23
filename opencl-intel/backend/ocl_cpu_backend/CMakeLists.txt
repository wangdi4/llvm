cmake_minimum_required(VERSION 2.8.7)

if( NOT DEFINED LLVM_PATH_FE )
    message( FATAL_ERROR "LLVM_PATH_FE is not specified. Please specify LLVM library location for backend using LLVM_PATH_FE parameter to CMAKE" )
endif()
set (LLVM_FE_INCLUDE_DIRS ${LLVM_PATH_FE}/include)

set(TARGET_NAME OclCpuBackEnd${BUILD_PLATFORM})
project(${TARGET_NAME})

if( CMAKE_SIZEOF_VOID_P EQUAL 4)
    # 32 bit
    set (PLATFORM 32)
else( CMAKE_SIZEOF_VOID_P EQUAL 8)
    # 64 bit
    set (PLATFORM 64)
endif( CMAKE_SIZEOF_VOID_P EQUAL 4)

set(OCL_CPU_BACKEND_DIR ${CMAKE_CURRENT_SOURCE_DIR})

#
# Settings for Linux compilation
#
if (NOT WIN32)
    #
    # Usage
    #     SET_LINUX_EXPORTS_FILE( TARGET FILE_NAME )
    # This function provides special linkage options for OCL Back-End. These options prevents exposure of function symbols
    # externally (except functions which are explicitly listed in version script

    function ( SET_LINUX_EXPORTS_FILE TARGET FILE_NAME )
            get_target_property( SOURCE_FILES ${TARGET_NAME} SOURCES )
            list( GET SOURCE_FILES 0 FIRST_SOURCE )
            set_source_files_properties( ${FIRST_SOURCE} PROPERTIES OBJECT_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${FILE_NAME} )
            set_property( TARGET ${TARGET_NAME} APPEND_STRING PROPERTY
                            LINK_FLAGS "-Wl,-Bsymbolic -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/${FILE_NAME}" )
    endfunction ( SET_LINUX_EXPORTS_FILE )

    set ( WARNING_LEVEL  "-pedantic -Wall -Wextra -Werror -Wno-unknown-pragmas -Wno-strict-aliasing -Wno-variadic-macros -Wno-long-long -Wno-unused-parameter -Wno-unused-function")

    # Compiler switches that CANNOT be modified during makefile generation
    set (ADD_COMMON_C_FLAGS  "-msse3 -mssse3 -fPIC -fdiagnostics-show-option -funsigned-bitfields")

    set (ADD_C_FLAGS         "${ADD_COMMON_C_FLAGS} -std=gnu99")
    set (ADD_CXX_FLAGS       "${ADD_COMMON_C_FLAGS}")

    set (ADD_C_FLAGS_DEBUG   "-O0 -ggdb -D _DEBUG")
    set (ADD_C_FLAGS_RELEASE "-O2 -ggdb ")


    # Compiler switches that CAN be modified during makefile generation and configuration-independent
    add_definitions( ${WARNING_LEVEL} )

    # Linker switches
    set (ADD_LINKER_FLAGS_DEBUG   )
    set (ADD_LINKER_FLAGS_RELEASE )

    # if we are cross-compiling for Android, let cmake know that execution of pthread test will fail on the host.
    if (ANDROID)
      set ( THREADS_PTHREAD_ARG "ANDROID-FAILED_TO_RUN" )
    endif (ANDROID)

    find_package(Threads)
    set( INIT_LINKER_FLAGS "${INIT_LINKER_FLAGS} ${CMAKE_THREAD_LIBS_INIT} -l${CMAKE_DL_LIBS} ${RT_LIB}" )

    # C switches
    set( CMAKE_C_FLAGS         "${CMAKE_C_FLAGS}         ${ADD_C_FLAGS}")
    set( CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG}   ${ADD_C_FLAGS_DEBUG}")
    set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${ADD_C_FLAGS_RELEASE}")

    # C++ switches
    set( CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS}         ${ADD_CXX_FLAGS}")
    set( CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   ${ADD_C_FLAGS_DEBUG}")
    set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${ADD_C_FLAGS_RELEASE}")

    # Linker switches - EXE
    set( CMAKE_EXE_LINKER_FLAGS           "${CMAKE_EXE_LINKER_FLAGS} ${INIT_LINKER_FLAGS}")
    set( CMAKE_EXE_LINKER_FLAGS_DEBUG     "${CMAKE_EXE_LINKER_FLAGS_DEBUG}   ${ADD_LINKER_FLAGS_DEBUG}")
    set( CMAKE_EXE_LINKER_FLAGS_RELEASE   "${CMAKE_EXE_LINKER_FLAGS_RELEASE} ${ADD_LINKER_FLAGS_RELEASE}")

    # Linker switches - Shared Lib
    set( CMAKE_SHARED_LINKER_FLAGS         "${CMAKE_SHARED_LINKER_FLAGS} ${INIT_LINKER_FLAGS}")
    set( CMAKE_SHARED_LINKER_FLAGS_DEBUG   "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}   ${ADD_LINKER_FLAGS_DEBUG}")
    set( CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} ${ADD_LINKER_FLAGS_RELEASE}")

endif (NOT WIN32)

#
# Settings for Windows compilation
#
if (WIN32)

    # Assember and its flags
    set( CMAKE_ASM_FLAGS /c /nologo /W3 /Zi /errorReport:prompt)
    set( CMAKE_ASM_INCLUDE_DIR_FLAG   /I )
    set( CMAKE_ASM_OUTPUT_NAME_FLAG   /Fo )
    set( CMAKE_ASM_COMPILER "$(VCInstallDir)bin/x86_amd64/ml64.exe")
    set( CMAKE_ASM_LINK_FLAG "/nologo")

endif (WIN32)


use_rtti(FALSE)

# Create target project
set (OCL_CPU_BACKEND_INCLUDE_FILES
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
    CompiledModule.h
    Compiler.h
    CompilerConfig.h
    ExecutionService.h
    ImageCallbackLibrary.h
    ImageCallbackManager.h
    ImageCallbackServices.h
    Kernel.h
    KernelProperties.h
    Program.h
    ProgramBuilder.h
    RuntimeService.h
    Serializer.h
    ServiceFactory.h
    StaticObjectLoader.h
    TargetDescription.h

    debuggingservicewrapper.h
    opencl_printf_ext.h
    Logger.h
    ocl_mutex.h
    ${BACKEND_ROOT_DIR}/utils/OclTune.h
    ${OCL_CPU_BACKEND_DIR}/export/plugin_interface.h
    ${BACKEND_ROOT_DIR}/ocl_cpu_debugging/export/icldebuggingservice.h
    ${BACKEND_ROOT_DIR}/../externals/itt/include/jitprofiling.h
  )

set (OCL_CPU_BACKEND_SOURCE_FILES
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
    ImageCallbackLibrary.cpp
    ImageCallbackManager.cpp
    ImageCallbackServices.cpp
    Kernel.cpp
    KernelProperties.cpp
    Logger.cpp
    Program.cpp
    Serializer.cpp
    ObjectCodeCache.cpp
    CPUSerializationService.cpp
    ObjectCodeContainer.cpp
    ProgramBuilder.cpp
    ServiceFactory.cpp
    TargetDescription.cpp
    cpu_builtin_functions.cpp
    debuggingservicewrapper.cpp
    dllmain.cpp
    ocl_debug_builtins.cpp
    ocl_mutex.cpp
    opencl20_ext_execution.cpp
    opencl_cpu_printf_ext.cpp
    opencl_printf_ext.cpp
    shared_builtin_functions.cpp
  )

if (INCLUDE_MIC_DEVICE)
  list(APPEND OCL_CPU_BACKEND_INCLUDE_FILES
    IMICCompilerConfig.h
    MICBuiltinLibrary.h
    MICCompileService.h
    MICCompiler.h
    MICCompilerConfig.h
    MICDeviceBackendFactory.h
    MICJITContainer.h
    MICKernel.h
    MICProgram.h
    MICProgramBuilder.h
    MICSerializationService.h
    ModuleJITHolder.h
  )

  list(APPEND OCL_CPU_BACKEND_SOURCE_FILES
    MICBuiltinLibrary.cpp
    MICCompileService.cpp
    MICCompiler.cpp
    MICCompilerConfig.cpp
    MICDeviceBackendFactory.cpp
    MICJITContainer.cpp
    MICKernel.cpp
    MICProgram.cpp
    MICProgramBuilder.cpp
    MICSerializationService.cpp
    ModuleJITHolder.cpp
    mic_builtin_functions.cpp
    opencl_mic_printf_ext.cpp
  )
endif (INCLUDE_MIC_DEVICE)

if (ENABLE_SDE AND INCLUDE_MIC_DEVICE)
  set(OCL_CPU_BACKEND_INCLUDE_FILES ${OCL_CPU_BACKEND_INCLUDE_FILES}
    ../ocl_mic_executor/MICDetect.h
    ../ocl_mic_executor/DynamicLibraryLoader.h
    MICExecutionService.h
  )
  set(OCL_CPU_BACKEND_SOURCE_FILES ${OCL_CPU_BACKEND_SOURCE_FILES}
    ../ocl_mic_executor/MICDetect.cpp
    ../ocl_mic_executor/DynamicLibraryLoader.cpp
    MICExecutionService.cpp
  )
endif()

if (WIN32)
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        add_definitions( -DRESOURCE_X64 )
    endif (CMAKE_SIZEOF_VOID_P EQUAL 8)
    # Resources
    set (OCL_CPU_BACKEND_RESOURCE_FILES llvm_backend.rc)
else (WIN32)
    set (OCL_CPU_BACKEND_RESOURCE_FILES)
endif (WIN32)

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    if (WIN32)
        set (ASM_SOURCES kernel_execute_64.asm)
        CREATE_ASM_RULES( ASM_ADD_TO_SOURCES_LIST ${ASM_SOURCES} )
    else (WIN32)
        set (ASM_ADD_TO_SOURCES_LIST linux64_utils.s)
    endif (WIN32)
endif (CMAKE_SIZEOF_VOID_P EQUAL 8)

# Add include directories
include_directories(
    ${LLVM_INCLUDE_DIRS}
    ${LLVM_FE_INCLUDE_DIRS}/cclang
    ${BACKEND_ROOT_DIR}
    ${OCL_CPU_BACKEND_DIR}
    ${OCL_CPU_BACKEND_DIR}/export
    ${BACKEND_ROOT_DIR}/../cl_api
    ${CL_API_HEADERS}
    ${BACKEND_ROOT_DIR}/ocl_cpu_debugging/export
    ${BACKEND_ROOT_DIR}/../utils/cl_sys_utils/export
    ${BACKEND_ROOT_DIR}/arch_headers
    ${BACKEND_ROOT_DIR}/vectorizer/Utils
    ${BACKEND_ROOT_DIR}/../externals/itt/include
    ${BACKEND_ROOT_DIR}/optimizer
    ${BACKEND_ROOT_DIR}/plugin_manager
    ${BACKEND_ROOT_DIR}/dynamic_lib
    ${BACKEND_ROOT_DIR}/utils
    ${BACKEND_ROOT_DIR}/metadata_api
    ${BACKEND_ROOT_DIR}/../utils/cache_binary_handler
    ${CMAKE_SOURCE_DIR}/utils/CLElfLib
    ${CMAKE_BINARY_DIR}/include/
    ${OCL_BINARY_DIR}
)
if (INCLUDE_MIC_DEVICE)
  include_directories(
    ${BACKEND_ROOT_DIR}/ocl_mic_executor
  )
endif (INCLUDE_MIC_DEVICE)

# Add additional link directories
if( LLVM_USE_INTEL_JITEVENTS )
    link_directories ( ${LLVM_INTEL_JITEVENTS_LIBDIR} )
endif( LLVM_USE_INTEL_JITEVENTS )

if( LLVM_USE_OPROFILE )
    link_directories ( ${LLVM_OPROFILE_INCDIR} )
endif( LLVM_USE_OPROFILE )

# Create DLL
add_library (${TARGET_NAME} SHARED
    ${OCL_CPU_BACKEND_INCLUDE_FILES}
    ${OCL_CPU_BACKEND_SOURCE_FILES}
    ${OCL_CPU_BACKEND_RESOURCE_FILES}
    ${ASM_ADD_TO_SOURCES_LIST}
)

if (NOT WIN32)
    SET_LINUX_EXPORTS_FILE( ${TARGET_NAME} ${TARGET_NAME}_linux_exports.txt )
endif (NOT WIN32)


set( BACKEND_PASSES
    LLVMBarrier
    ImplicitArgsAnalysis
    LLVMVectorizer
    AddImplicitArgs
    ChannelPipeTransformation
    PipeSupport
    OclFunctionAttrs
    BuiltInFuncImport
    BuiltinLibInfo
    BlockToFuncPtr
    DebugInfoPass
    DetectRecursion
    InstToFuncCall
    GenericAddressResolution
    LocalBuffers
    LocalBuffAnalysis
    ModuleCleanup
    PrepareKernelArgs
    PreventDivCrashes
    PrintIRPass
    ProfilingInfoPass
    RelaxedPass
    ResolveWICall
    ShiftZeroUpperBits
    BuiltinCallToInst
    UndefExternalFuncs
    KernelInfo
    Prefetch
    SubGroupAdaptation
    LinearIdResolver
    Materialize
    ReduceAlignment
    DetectFunctionPtrCalls
    CloneBlockInvokeFuncToKernel
    ResolveBlockToStaticCall
    PatchCallbackArgs
    SmartGVN
    SinCosFold
    PreLegalizeBools
    OCLAliasAnalysis
    SPIR20BlocksToObjCBlocks
    PrintfArgumentsPromotion
    FMASplitter
)

set(UTILS_LIBS
    # Utils lib
    OclPluginManager
    OclOptimizer
)
if (INCLUDE_MIC_DEVICE)
  list (APPEND UTILS_LIBS LLVMMICCodeGenerationEngine)
endif (INCLUDE_MIC_DEVICE)

target_link_libraries (${TARGET_NAME}
    ${UTILS_LIBS}
    # internal passes
    ${BACKEND_PASSES}

    LoopUtils
    OclBackendUtils
    reflection_module
    name_mangle
    dynamic_load
    cl_sys_utils
    metadataapi
    CacheBinaryHandler
    CLElfLib

    # LLVM LIBS
    ${LLVM_MODULE_LIBS}
    )

if(ANDROID)
    # dynamic link with STL library
     target_link_libraries(${TARGET_NAME} c++)
endif(ANDROID)

set( CMAKE_CXX_FLAGS_DEBUG "-DOCL_DEV_BACKEND_PLUGINS ${CMAKE_CXX_FLAGS_DEBUG}")

add_definitions( -DOclCpuBackEnd_EXPORTS )
if (KNC_CARD)
  add_definitions( -DKNC_CARD )
endif (KNC_CARD)

if (ENABLE_SDE)
    message(STATUS "-- Enabling SDE version ... ")
    add_definitions( -DENABLE_SDE )
endif (ENABLE_SDE)

if (WIN32)
    get_target_property(RT_OUTPUT_DIRECTORY ${TARGET_NAME} RUNTIME_OUTPUT_DIRECTORY)
    file(TO_NATIVE_PATH ${RT_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/${TARGET_NAME}_stripped.pdb PDB_NAME)
    if (${MSVC_VERSION} EQUAL 1500)
        # Visual Studio 2008
        set_target_properties(${TARGET_NAME} PROPERTIES
                                LINK_FLAGS_RELEASE  "${LINK_FLAGS_RELEASE} /PDBSTRIPPED:${PDB_NAME}"
                                LINK_FLAGS_DEBUG    "${LINK_FLAGS_DEBUG} /PDBSTRIPPED:${PDB_NAME}"
                             )
    else (${MSVC_VERSION} EQUAL 1500)
        # Visual Studio 2010 (assumed if not Visual Studio 2008)
        # This is a fix due to a bug in CMake, Does not add the flag /DEBUG to the linker flags in Release mode.
        # The /DEBUG flag is required in order to create stripped pdbs.
        set_target_properties(${TARGET_NAME} PROPERTIES
                                LINK_FLAGS_RELEASE  "${LINK_FLAGS_RELEASE} /DEBUG /PDBSTRIPPED:${PDB_NAME}"
                                LINK_FLAGS_DEBUG    "${LINK_FLAGS_DEBUG} /PDBSTRIPPED:${PDB_NAME}"
                                )
    endif (${MSVC_VERSION} EQUAL 1500)
    if (INSTALL_PDBS)
        install(FILES ${OCL_TOOLS_BINARY_DIR}/\${BUILD_TYPE}/${TARGET_NAME}.pdb DESTINATION bin)
    endif(INSTALL_PDBS)
    install(FILES ${OCL_TOOLS_BINARY_DIR}/\${BUILD_TYPE}/${TARGET_NAME}_stripped.pdb DESTINATION bin)
endif(WIN32)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME} )

install( TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION bin)
