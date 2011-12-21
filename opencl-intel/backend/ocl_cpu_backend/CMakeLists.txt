set(TARGET_NAME OclCpuBackEnd)
set(TARGET_UTILS_NAME OclUtils)

project(${TARGET_NAME})

cmake_minimum_required(VERSION 2.8.4)

# provides an option for supporting SDE Debug Tracing
set(ENABLE_DEBUG_TRACE OFF)

if( CMAKE_SIZEOF_VOID_P EQUAL 4)
	# 32 bit
	set (PLATFORM 32)
else( CMAKE_SIZEOF_VOID_P EQUAL 8)
	# 64 bit
	set (PLATFORM 64)
endif( CMAKE_SIZEOF_VOID_P EQUAL 4)

set(OCL_CPU_BACKEND_DIR ${CMAKE_CURRENT_SOURCE_DIR})
# The directory 'cl_api' below is used by OCL BE only when compiling outside of OCL SDK, otherwise OCL SDK overrides that setting
set(OCL_CPU_BACKEND_IMPORT_DIRS 
  ${BACKEND_ROOT_DIR}/../cl_api
  ${OCL_CPU_BACKEND_DIR}/import 
  ${BACKEND_ROOT_DIR}/ocl_cpu_debugging/export
  ${OCL_CPU_BACKEND_DIR}/../vectorizer/VolcanoWrapper
  ${OCL_CPU_BACKEND_DIR}/../arch_headers)
set(OCL_CPU_BACKEND_EXPORT_DIR ${OCL_CPU_BACKEND_DIR}/export)
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
	SET(CMAKE_SKIP_BUILD_RPATH                  TRUE)   # do not add pointers to the build tree
	SET(CMAKE_BUILD_WITH_INSTALL_RPATH          TRUE)   # build rpath as if already installed
	SET(CMAKE_INSTALL_RPATH                     "$ORIGIN") # the rpath to use - search through installation dir only
	SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH       FALSE)  # do not use static link paths as rpath

# Warning level
set ( WARNING_LEVEL  "-pedantic -Wall -Wextra -Werror -Wno-unknown-pragmas -Wno-strict-aliasing -Wno-variadic-macros -Wno-long-long -Wno-unused-parameter")

# Assembler setup - use private rules
set( CMAKE_ASM_FLAGS               --64 ) # do not quote this!!!!
set( CMAKE_ASM_INCLUDE_DIR_FLAG    -I )
set( CMAKE_ASM_OUTPUT_NAME_FLAG    -o )
if (CVCC_ASM_PATH)
  set(CMAKE_ASM_COMPILER         ${CVCC_ASM_PATH})
else(CVCC_ASM_PATH)
  message(STATUS "Using system assembler, check that its version is 4.3.x")
  set( CMAKE_ASM_COMPILER             as )
endif(CVCC_ASM_PATH)

# Compiler switches that CANNOT be modified during makefile generation
set (ADD_COMMON_C_FLAGS  "-msse3 -mssse3 -fPIC -fdiagnostics-show-option -funsigned-bitfields")

set (ADD_C_FLAGS         "${ADD_COMMON_C_FLAGS} -std=gnu99")
set (ADD_CXX_FLAGS       "${ADD_COMMON_C_FLAGS} ")

set (ADD_C_FLAGS_DEBUG   "-O0 -ggdb -D _DEBUG")
set (ADD_C_FLAGS_RELEASE "-O2 -g0 ")

# Compiler switches that CAN be modified during makefile generation and configuration-independent
add_definitions( ${WARNING_LEVEL} )

# Linker switches
set (INIT_LINKER_FLAGS        "-pie -Wl,--enable-new-dtags") # --enable-new-dtags sets RUNPATH to the same value as RPATH
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

# Target directory
set(OCL_CPU_BACKEND_LIB_DIR ${OCL_CPU_BACKEND_DIR}/lib/Linux${PLATFORM})

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
SET(CMAKE_SKIP_BUILD_RPATH                  TRUE)   # do not add pointers to the build tree
SET(CMAKE_BUILD_WITH_INSTALL_RPATH          TRUE)   # build rpath as if already installed
SET(CMAKE_INSTALL_RPATH                     "$ORIGIN") # the rpath to use - search through installation dir only
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH       FALSE)  # do not use static link paths as rpath

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


# Target directory
set(OCL_CPU_BACKEND_LIB_DIR ${OCL_CPU_BACKEND_DIR}/lib/Win${PLATFORM})

# Linker flags
set(CMAKE_SHARED_LINKER_FLAGS  /NODEFAULTLIB:LIBCMTD)

endif (WIN32)


#
# Usage
#     CREATE_ASM_RULES( <ADD_TO_SOURCES_LIST_VAR> ...<asm-files-list>...)
#
# Creates ADD_TO_SOURCES_LIST_VAR that should be added to the source files list
#
macro( CREATE_ASM_RULES ADD_TO_SOURCES_LIST_VAR )

    if (${ARGC} GREATER  1)
        foreach( FILE ${ARGN} )
            get_filename_component( FILE_NAME ${FILE} NAME_WE )
            set( BIN_DIR  ${CMAKE_CURRENT_BINARY_DIR}/${INSTALL_SUBDIR} )
            set( OBJ_FILE ${BIN_DIR}/${FILE_NAME}${CMAKE_C_OUTPUT_EXTENSION} )
            set( SRC_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${FILE} )

            if (CMAKE_ASM_OUTPUT_NAME_FLAG)
								set( OBJ_OUTPUT_NAME_FLAG ${CMAKE_ASM_OUTPUT_NAME_FLAG} ${FILE_NAME}${CMAKE_C_OUTPUT_EXTENSION} )
						endif (CMAKE_ASM_OUTPUT_NAME_FLAG)

            add_custom_command(OUTPUT ${OBJ_FILE}
                               COMMAND ${CMAKE_COMMAND} -E make_directory ${BIN_DIR}
                               COMMAND ${CMAKE_COMMAND} -E chdir ${BIN_DIR}
                                       ${CMAKE_ASM_COMPILER} ${CMAKE_ASM_FLAGS}
                                                             ${CMAKE_ASM_INCLUDE_DIR_FLAG} ${CMAKE_CURRENT_SOURCE_DIR}
                                                             ${OBJ_OUTPUT_NAME_FLAG}
                                                             ${SRC_FILE}
                               MAIN_DEPENDENCY ${SRC_FILE}
                               VERBATIM
                               )

            list(APPEND OBJ_FILES ${OBJ_FILE})
            list(APPEND SRC_FILES ${SRC_FILE})
        endforeach( FILE )
    endif (${ARGC} GREATER 1)

    if (DEFINED OBJ_FILES)
        set (${ADD_TO_SOURCES_LIST_VAR} ${SRC_FILES} ${OBJ_FILES})
        set_source_files_properties( ${OBJ_FILES} PROPERTIES GENERATED TRUE EXTERNAL_OBJECT TRUE )
    endif(DEFINED OBJ_FILES)

endmacro( CREATE_ASM_RULES )

# Create target utils project
set (OCL_UTILS_INCLUDE_FILES
	AddImplicitArgs.h
	CompilationUtils.h
	CPUDetect.h
	DynamicLib.h
	exceptions.h
	ExplicitArgument.h
	ExplicitLocalMemArgument.h
	ExplicitGlobalMemArgument.h
	FunctionArgument.h
	IArgument.h
	ImplicitArgProperties.h
	ImplicitArgsUtils.h
	ImplicitArgument.h
	LocalBuffers.h
	LocalBuffersAnalysis.h
	PrepareKernelArgs.h
	ResolveWICall.h
	SystemInfo.h
	TLLVMKernelInfo.h
	TypeAlignment.h
	UndefinedExternalFunctions.h
  )
  
set (OCL_UTILS_SOURCE_FILES
	AddImplicitArgs.cpp
	BuiltInFunctionImport.cpp
	CompilationUtils.cpp
	CPUDetect.cpp
	DynamicLib.cpp
	FunctionArgument.cpp
	ImplicitArgsUtils.cpp
	LocalBuffers.cpp
	LocalBuffersAnalysis.cpp
	PrepareKernelArgs.cpp
	ResolveWICall.cpp
	SystemInfo.cpp
	TypeAlignment.cpp
	UndefinedExternalFunctions.cpp
  )

# Create target project
    
set (OCL_CPU_BACKEND_INCLUDE_FILES
    ProcessorDetect.h
    IAbstractBackendFactory.h
    IDynamicFunctionsResolver.h
    CPUDeviceBackendFactory.h
    MICDeviceBackendFactory.h
    ${OCL_CPU_BACKEND_EXPORT_DIR}/plugin_interface.h
    TargetDescription.h
    exceptions.h
    Logger.h
    Compiler.h
    CPUCompiler.h
    MICCompiler.h
    plugin_manager.h
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
    CompilationUtils.h
    CPUProgramBuilder.h
    MICProgramBuilder.h
    Optimizer.h
    BuiltinModuleManager.h
    BuiltinModule.h
    CPUBuiltinLibrary.h
    MICBuiltinLibrary.h
    Program.h
    MICProgram.h
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
    TypeAlignment.h
    PrintIRPass.h
    ImageCallbackLibrary.h
    ImageCallbackManager.h
    ImageCallbackServices.h
    debuggingservicewrapper.h
    ${BACKEND_ROOT_DIR}/ocl_cpu_debugging/export/icldebuggingservice.h
  )

set (OCL_CPU_BACKEND_SOURCE_FILES
    ProcessorDetect.cpp
    CPUDeviceBackendFactory.cpp
    MICDeviceBackendFactory.cpp
    builtin_functions.cpp
    TargetDescription.cpp
    dllmain.cpp
    InstToFuncCall.cpp
    opencl_printf_ext.cpp
    RelaxedPass.cpp
    Logger.cpp
    plugin_manager.cpp
    ServiceFactory.cpp
    CompileService.cpp
    CPUCompileService.cpp
    MICCompileService.cpp
    ExecutionService.cpp
    CPUExecutionService.cpp
    MICSerializationService.cpp
    BackendConfiguration.cpp
    ProgramBuilder.cpp
    CompilationUtils.cpp
    Compiler.cpp
    CPUCompiler.cpp
    MICCompiler.cpp
    CPUProgramBuilder.cpp
    MICProgramBuilder.cpp
    Optimizer.cpp
    BuiltinModuleManager.cpp
    BuiltinModule.cpp
    CPUBuiltinLibrary.cpp
    MICBuiltinLibrary.cpp
    Program.cpp
    MICProgram.cpp
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
    TypeAlignment.cpp
    PrintIRPass.cpp
    ImageCallbackLibrary.cpp
    ImageCallbackManager.cpp
    ImageCallbackServices.cpp
    debuggingservicewrapper.cpp
    ocl_debug_builtins.cpp
    debuginfopass.cpp
  )
 if (WIN32)	
 set (OCL_CPU_BACKEND_RESOURCE_FILES
	# Resources
	llvm_backend.rc
	)
  if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        #SVML call wrapper is specific for Win64 calling conventions 
	    list (APPEND OCL_CPU_BACKEND_SOURCE_FILES WrapSvml.cpp)
  endif (CMAKE_SIZEOF_VOID_P EQUAL 8)
else (WIN32)
set (OCL_CPU_BACKEND_RESOURCE_FILES
  )
endif (WIN32)	
	
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
if (WIN32)
    set (ASM_SOURCES kernel_execute_64.asm svml_thunk.asm)
else (WIN32)
    set (ASM_SOURCES linux64_utils.s)
endif (WIN32)
CREATE_ASM_RULES( ASM_ADD_TO_SOURCES_LIST ${ASM_SOURCES} )
endif (CMAKE_SIZEOF_VOID_P EQUAL 8)

# Add include directories
include_directories(${OCL_CPU_BACKEND_IMPORT_DIRS} 
	${OCL_CPU_BACKEND_EXPORT_DIR} 
	)

# Add additional lib direcories
link_directories (
	${OCL_CPU_BACKEND_LIB_DIR} 
) 

# Create DLL
add_library (${TARGET_NAME} SHARED 
	${OCL_CPU_BACKEND_INCLUDE_FILES}
	${OCL_CPU_BACKEND_SOURCE_FILES}
	${OCL_CPU_BACKEND_RESOURCE_FILES}
	${ASM_ADD_TO_SOURCES_LIST}
	)
add_library(${TARGET_UTILS_NAME}
    ${OCL_UTILS_INCLUDE_FILES}
	${OCL_UTILS_SOURCE_FILES}
	${ASM_ADD_TO_SOURCES_LIST}
)

if (NOT WIN32)
	SET_LINUX_EXPORTS_FILE( ${TARGET_NAME} ${TARGET_NAME}_linux_exports.txt )
endif (NOT WIN32)

#For OCL SDK - Shared libraries should be in the same location as executables
if ((NOT WIN32) AND (DEFINED RUNTIME_OUTPUT_DIRECTORY))
	get_target_property(RT_OUTPUT_DIRECTORY ${TARGET_NAME} RUNTIME_OUTPUT_DIRECTORY)
	set_target_properties(${TARGET_NAME} PROPERTIES
				LIBRARY_OUTPUT_DIRECTORY ${RT_OUTPUT_DIRECTORY})
endif((NOT WIN32) AND (DEFINED RUNTIME_OUTPUT_DIRECTORY))

if(WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 8)
    #SVML call wrapper is specific for Win64 calling conventions 
    set_target_properties( ${TARGET_NAME} PROPERTIES LINK_FLAGS "/EXPORT:SvmlThunk")
endif(WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 8)

 set (LLVM_LIBS
	
	# LLVM libs
	LLVMMICCodeGenerationEngine
	LLVMMICModuleJIT
	LLVMLinker
	LLVMX86AsmParser
	LLVMX86AsmPrinter
	LLVMX86CodeGen
	LLVMX86Disassembler
	LLVMX86Info
	LLVMTransformUtils
	LLVMCore
	LLVMTarget
	LLVMSystem
	LLVMExecutionEngine
	LLVMJIT
	LLVMCodeGen
	LLVMBitReader
	LLVMBitWriter
	LLVMSupport
	LLVMAnalysis
	LLVMAsmPrinter
	LLVMInstCombine	
	LLVMipo
	LLVMMC
	LLVMScalarOpts
	LLVMSelectionDAG
	LLVMVectorizer
	LLVMBarrier
	LLVMipa
	LLVMX86AsmPrinter
	LLVMMC
	)
# the following targets are necessary for successful build of OCL Back-End shared library outside of OCL SDK framework
# they are about C/C++ backends for LLVM (not to be confused with OCL backend)
if ((NOT WIN32) AND (OCL_BACKEND_STAND_ALONE))
  list(APPEND LLVM_LIBS
        LLVMCBackend
        LLVMCBackendInfo
        LLVMCppBackend
        LLVMCppBackendInfo
      )
endif ((NOT WIN32) AND (OCL_BACKEND_STAND_ALONE))
	
# Link with appropriate libs
if (WIN32)
    set( SOCKETLIB ws2_32 )
else (WIN32)
    set( SOCKETLIB "" )
endif (WIN32)

set( JITPROFLIB JITProfiling)

target_link_libraries (${TARGET_NAME} 
	# Internal libs
	${JITPROFLIB} 
	
	# Utils lib
	${TARGET_UTILS_NAME}
	
	# LLVM LIBS
	${LLVM_LIBS}
	) 


add_dependencies(${TARGET_NAME} 
	# Utils lib
	${TARGET_UTILS_NAME}
	
	# LLVM PROJECTS
	${LLVM_LIBS}
	) 
	

add_definitions( -DOCL_DEV_BACKEND_PLUGINS )

if (ENABLE_DEBUG_TRACE)
    message("-- Enabling SDE Debug Trace ... ")
    add_definitions( -DENABLE_SDE_DEBUG_TRACE )
endif (ENABLE_DEBUG_TRACE)

install( TARGETS ${TARGET_NAME} ${TARGET_UTILS_NAME} 
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION bin)
