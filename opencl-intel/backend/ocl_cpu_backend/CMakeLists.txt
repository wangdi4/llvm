if( NOT DEFINED LLVM_PATH_FE )
    message( FATAL_ERROR "LLVM_PATH_FE is not specified. Please specify LLVM library location for backend using LLVM_PATH_FE parameter to CMAKE" )
endif()
set (LLVM_FE_INCLUDE_DIRS ${LLVM_PATH_FE}/include)

set(TARGET_NAME OclCpuBackEnd${BUILD_PLATFORM})
project(${TARGET_NAME})
add_subdirectory(emutls)

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
    get_filename_component( CMAKE_ASM_COMPILER_DIR ${CMAKE_CXX_COMPILER} DIRECTORY )
    set ( CMAKE_ASM_COMPILER "${CMAKE_ASM_COMPILER_DIR}/ml64.exe" )
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
    EyeQBuiltinLibrary.h
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
    ${OCL_SOURCE_DIR}/externals/itt/include/jitprofiling.h
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
    EyeQBuiltinLibrary.cpp
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
  )

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    if (WIN32)
        set (ASM_SOURCES kernel_execute_64.asm)
        CREATE_ASM_RULES( ASM_ADD_TO_SOURCES_LIST ${ASM_SOURCES} )
    else (WIN32)
        set (ASM_ADD_TO_SOURCES_LIST linux64_utils.s)
    endif (WIN32)
endif (CMAKE_SIZEOF_VOID_P EQUAL 8)

# Add include directories
set (INCLUDE_DIRS
    ${LLVM_INCLUDE_DIRS}
    ${LLVM_FE_INCLUDE_DIRS}/cclang
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
    ${BACKEND_ROOT_DIR}/passes/ChannelPipeTransformation
    ${CMAKE_BINARY_DIR}/include/
    ${OCL_BINARY_DIR}
)

# Add additional link directories
if( LLVM_USE_INTEL_JITEVENTS )
    link_directories ( ${LLVM_INTEL_JITEVENTS_LIBDIR} )
endif( LLVM_USE_INTEL_JITEVENTS )

if( LLVM_USE_OPROFILE )
    link_directories ( ${LLVM_OPROFILE_INCDIR} )
endif( LLVM_USE_OPROFILE )

set( BACKEND_PASSES
    LLVMBarrier
    ImplicitArgsAnalysis
    LLVMVectorizer
    AddImplicitArgs
    AddTLSGlobals
    AutorunReplicator
    ChannelPipeTransformation
    ChannelsUsageAnalysis
    PipeIOTransformation
    CleanupWrappedKernels
    CoerceTypes
    PipeOrdering
    PipeSupport
    OclFunctionAttrs
    BuiltInFuncImport
    BuiltinLibInfo
    DebugInfoPass
    DetectRecursion
    InfiniteLoopCreator
    InstToFuncCall
    InternalizeNonKernelFunc
    InternalizeGlobalVariables
    GenericAddressResolution
    LocalBuffers
    LocalBuffAnalysis
    PrepareKernelArgs
    PreventDivCrashes
    PrintIRPass
    ProfilingInfoPass
    RelaxedPass
    ResolveWICall
    ResolveSubGroupWICall
    ShiftZeroUpperBits
    BuiltinCallToInst
    UndefExternalFuncs
    KernelInfo
    Prefetch
    SubGroupAdaptation
    LinearIdResolver
    ResolveBlockToStaticCall
    LLVMEqualizer
    PatchCallbackArgs
    SinCosFold
    SmartGVN
    StripIntelIP
    OCLAliasAnalysis
    PrintfArgumentsPromotion
    SYCLPipesHack
    OCLLoopIdiomRecognize
)

set(UTILS_LIBS
    # Utils lib
    OclPluginManager
    OclOptimizer
)

# Create DLL
add_opencl_library (${TARGET_NAME} SHARED
    ${OCL_CPU_BACKEND_INCLUDE_FILES}
    ${OCL_CPU_BACKEND_SOURCE_FILES}
    ${ASM_ADD_TO_SOURCES_LIST}

    INCLUDE_DIRS ${INCLUDE_DIRS}

    LINK_LIBS
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
      emutls

      # LLVM LIBS
      ${LLVM_MODULE_LIBS}

    RC_TEMPLATE DEFAULT
)

SET_LINUX_EXPORTS_FILE( ${TARGET_NAME} ${TARGET_NAME}_linux_exports.txt )

set( CMAKE_CXX_FLAGS_DEBUG "-DOCL_DEV_BACKEND_PLUGINS ${CMAKE_CXX_FLAGS_DEBUG}")

add_definitions( -DOclCpuBackEnd_EXPORTS )

if (ENABLE_SDE)
    message(STATUS "-- Enabling SDE version ... ")
    add_definitions( -DENABLE_SDE )
endif (ENABLE_SDE)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME} )
