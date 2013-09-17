cmake_minimum_required(VERSION 2.8.7)

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
# the following flag serves as a differentiator between OCL Back-End standalone build and its build within OCL SDK framework
option(OCL_BACKEND_STAND_ALONE "CBackend Enabled" ON)

#
# Settings for Linux compilation
#
if (NOT WIN32)
    #
    # Usage
    #     SET_LINUX_EXPORTS_FILE( TARGET FILE_NAME )
    # This function provides special linkage options for OCL Back-End. These options prevents exposure of function symbols
    # externally (except functions which are explicitely listed in version script

    function ( SET_LINUX_EXPORTS_FILE TARGET FILE_NAME )
            get_target_property( SOURCE_FILES ${TARGET_NAME} SOURCES )
            list( GET SOURCE_FILES 0 FIRST_SOURCE )
            set_source_files_properties( ${FIRST_SOURCE} PROPERTIES OBJECT_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${FILE_NAME} )
            set_property( TARGET ${TARGET_NAME} APPEND PROPERTY
                            LINK_FLAGS "-Wl,-Bsymbolic -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/${FILE_NAME}" )
    endfunction ( SET_LINUX_EXPORTS_FILE )

    # configure immediate dependencies search path at run time for non-standard libs
    set(CMAKE_SKIP_BUILD_RPATH                  TRUE)   # do not add pointers to the build tree
    set(CMAKE_BUILD_WITH_INSTALL_RPATH          TRUE)   # build rpath as if already installed
    set(CMAKE_INSTALL_RPATH                     "$ORIGIN") # the rpath to use - search through installation dir only
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH       FALSE)  # do not use static link paths as rpath

set ( WARNING_LEVEL  "-pedantic -Wall -Wextra -Werror -Wno-unknown-pragmas -Wno-strict-aliasing -Wno-variadic-macros -Wno-long-long -Wno-unused-parameter -Wno-unused-function")

    # Assembler setup - use private rules
    # set( CMAKE_ASM_FLAGS               --64 ) # do not quote this!!!!
    set( CMAKE_ASM_INCLUDE_DIR_FLAG    -I )
    set( CMAKE_ASM_OUTPUT_NAME_FLAG    -o )
if( NOT ANDROID )
  if (CVCC_ASM_PATH)
    set(CMAKE_ASM_COMPILER         ${CVCC_ASM_PATH})
  else(CVCC_ASM_PATH)
    message(STATUS "Using system assembler, check that its version is 4.3.x")
    set( CMAKE_ASM_COMPILER             as )
  endif(CVCC_ASM_PATH)
endif( NOT ANDROID )

    # Compiler switches that CANNOT be modified during makefile generation
    set (ADD_COMMON_C_FLAGS  "-msse3 -mssse3 -fPIC -fdiagnostics-show-option -funsigned-bitfields")

    set (ADD_C_FLAGS         "${ADD_COMMON_C_FLAGS} -std=gnu99")
    set (ADD_CXX_FLAGS       "${ADD_COMMON_C_FLAGS}")

    set (ADD_C_FLAGS_DEBUG   "-O0 -ggdb -D _DEBUG")
    set (ADD_C_FLAGS_RELEASE "-O2 -ggdb ")


    # Compiler switches that CAN be modified during makefile generation and configuration-independent
    add_definitions( ${WARNING_LEVEL} )

    # Linker switches
    set (INIT_LINKER_FLAGS        "-pie -Wl,--enable-new-dtags") # --enable-new-dtags sets RUNPATH to the same value as RPATH
    set (ADD_LINKER_FLAGS_DEBUG   )
    set (ADD_LINKER_FLAGS_RELEASE )

    include(FindThreads)
set( INIT_LINKER_FLAGS "${INIT_LINKER_FLAGS} ${CMAKE_THREAD_LIBS_INIT} -l${CMAKE_DL_LIBS} ${RT_LIB}" )


    # embed RPATH and RUNPATH to the binaries that assumes that everything is installed in the same directory
    #
    # Description:
    #   RPATH is used to locate dynamically load shared libraries/objects (DLLs) for the non-standard OS
    #   locations without need of relinking DLLs during installation. The algorithm is the following:
    #
    #     1. If RPATH is present in the EXE/DLL and RUNPATH is NOT present, search through it.
    #     2. If LD_LIBRARY_PATH env variable is present, search through it
    #     3. If RUNPATH is present in the EXE/DLL, search through it
    #     4. Search through locations, configured by system admin and cached in /etc/ld.so.cache
    #     5. Search through /lib and /usr/lib
    #
    #   RUNPATH influences only the immediate dependencies, while RPATH influences the whole subtree of dependencies
    #   RPATH is concidered deprecated in favor of RUNPATH, but RUNPATH does not supported by some Linux systems.
    #   If RUNPATH is not supported, system loader may report error - remove "-Wl,--enable-new-dtags" above to
    #   disable RUNPATH generation.
    #
    #   If RPATH or RUNPATH contains string $ORIGIN it is substituted by the full path to the containing EXE/DLL.
    #   Security issue 1: if EXE/DLL is marked as set-uid or set-gid, $ORIGIN is ignored.
    #   Security issue 2: if RPATH/RUNPATH references relative subdirs, intruder may fool it by using softlinks
    #
    set(CMAKE_SKIP_BUILD_RPATH                  TRUE)   # do not add pointers to the build tree
    set(CMAKE_BUILD_WITH_INSTALL_RPATH          TRUE)   # build rpath as if already installed
    set(CMAKE_INSTALL_RPATH                     "$ORIGIN") # the rpath to use - search through installation dir only
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH       FALSE)  # do not use static link paths as rpath

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
    IAbstractBackendFactory.h
    IDynamicFunctionsResolver.h
    CPUDeviceBackendFactory.h
    MICDeviceBackendFactory.h
    ${OCL_CPU_BACKEND_DIR}/export/plugin_interface.h
    TargetDescription.h
    Logger.h
    Compiler.h
    CPUCompiler.h
    MICCompiler.h
    ServiceFactory.h
    CompileService.h
    CPUCompileService.h
    MICCompileService.h
    MICSerializationService.h
    Serializer.h
    ExecutionService.h
    CPUExecutionService.h
    BackendConfiguration.h
    CompilerConfig.h
    MICCompilerConfig.h
    ProgramBuilder.h
    CPUProgramBuilder.h
    MICProgramBuilder.h
    BuiltinModuleManager.h
    BuiltinModule.h
    CPUBuiltinLibrary.h
    MICBuiltinLibrary.h
    Program.h
    MICProgram.h
    CPUProgram.h
    BitCodeContainer.h
    ProgramContainerMemoryBuffer.h
    ModuleJITHolder.h
    Kernel.h
    MICKernel.h
    KernelProperties.h
    MICKernelProperties.h
    CPUJITContainer.h
    MICJITContainer.h
    Binary.h
    Executable.h
    CPUExecutable.h
    MICExecutable.h
    ImageCallbackLibrary.h
    ImageCallbackManager.h
    ImageCallbackServices.h
    debuggingservicewrapper.h
    opencl_printf_ext.h
    ExtendedExecutionContext.h
    BlockLiteral.h
    RuntimeService.h
    IBlockToKernelMapper.h
    CPUBlockToKernelMapper.h
    ${BACKEND_ROOT_DIR}/ocl_cpu_debugging/export/icldebuggingservice.h
    ${BACKEND_ROOT_DIR}/../externals/itt/include/jitprofiling.h
  )

set (OCL_CPU_BACKEND_SOURCE_FILES
    CPUDeviceBackendFactory.cpp
    MICDeviceBackendFactory.cpp
    shared_builtin_functions.cpp
    cpu_builtin_functions.cpp
    mic_builtin_functions.cpp
    opencl20_ext_execution.cpp
    TargetDescription.cpp
    dllmain.cpp
    opencl_printf_ext.cpp
    opencl_cpu_printf_ext.cpp
    opencl_mic_printf_ext.cpp
    Logger.cpp
    ServiceFactory.cpp
    CompileService.cpp
    CPUCompileService.cpp
    MICCompileService.cpp
    ExecutionService.cpp
    CPUExecutionService.cpp
    MICSerializationService.cpp
    BackendConfiguration.cpp
    ProgramBuilder.cpp
    Compiler.cpp
    CPUCompiler.cpp
    MICCompiler.cpp
    CPUProgramBuilder.cpp
    MICProgramBuilder.cpp
    BuiltinModuleManager.cpp
    BuiltinModule.cpp
    CPUBuiltinLibrary.cpp
    MICBuiltinLibrary.cpp
    Program.cpp
    MICProgram.cpp
    CPUProgram.cpp
    BitCodeContainer.cpp
    ProgramContainerMemoryBuffer.cpp
    Kernel.cpp
    MICKernel.cpp
    KernelProperties.cpp
    MICKernelProperties.cpp
    CPUJITContainer.cpp
    MICJITContainer.cpp
    ModuleJITHolder.cpp
    Binary.cpp
    Executable.cpp
    CPUExecutable.cpp
    MICExecutable.cpp
    ImageCallbackLibrary.cpp
    ImageCallbackManager.cpp
    ImageCallbackServices.cpp
    debuggingservicewrapper.cpp
    ocl_debug_builtins.cpp
    CPUBlockToKernelMapper.cpp
  )

if (ENABLE_SDE)
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
    else (WIN32)
        set (ASM_SOURCES linux64_utils.s)
    endif (WIN32)
    CREATE_ASM_RULES( ASM_ADD_TO_SOURCES_LIST ${ASM_SOURCES} )
endif (CMAKE_SIZEOF_VOID_P EQUAL 8)

# Add include directories
include_directories(
    ${BACKEND_ROOT_DIR}
    ${OCL_CPU_BACKEND_DIR}
    ${OCL_CPU_BACKEND_DIR}/export
    ${BACKEND_ROOT_DIR}/../cl_api
    ${BACKEND_ROOT_DIR}/ocl_cpu_debugging/export
    ${BACKEND_ROOT_DIR}/../utils/cl_sys_utils/export
    ${BACKEND_ROOT_DIR}/arch_headers
    ${BACKEND_ROOT_DIR}/vectorizer/Utils
    ${BACKEND_ROOT_DIR}/ocl_mic_executor
    ${BACKEND_ROOT_DIR}/plugin_manager
    ${BACKEND_ROOT_DIR}/../externals/itt/include
    ${BACKEND_ROOT_DIR}/optimizer
    ${BACKEND_ROOT_DIR}/plugin_manager
    ${BACKEND_ROOT_DIR}/dynamic_lib
    ${BACKEND_ROOT_DIR}/utils
    ${BACKEND_ROOT_DIR}/metadata_api
    ${BACKEND_ROOT_DIR}/metadata_api/gen
    ${OCL_BINARY_DIR}
)

link_directories( 
            ${LLVM_LIBRARY_DIR}
            )

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
    LLVMVectorizer
    LLVMBarrier
    AddImplicitArgs
    BuiltInFuncImport
    DebugInfoPass
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
    ShuffleCallToInst
    UndefExternalFuncs
    KernelInfo
    Prefetch
    LinearIdResolver
    Materialize
    DetectFunctionPtrCalls
    CloneBlockInvokeFuncToKernel
    ResolveBlockToStaticCall
)

target_link_libraries (${TARGET_NAME}
    # Utils lib
    OclPluginManager
    OclOptimizer
    LLVMMICCodeGenerationEngine
    
    # internal passes
    ${BACKEND_PASSES}

    OclBackendUtils
    reflection_module
    name_mangle
    dynamic_load
    cl_sys_utils
    metadataapi

    # LLVM LIBS
    ${LLVM_MODULE_LIBS}
    )

if(NOT WIN32)
    if(NOT ANDROID)
        target_link_libraries(${TARGET_NAME} pthread rt)
    endif(NOT ANDROID)
endif(NOT WIN32)

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
