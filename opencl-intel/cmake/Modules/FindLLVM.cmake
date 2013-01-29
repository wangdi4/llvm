# Find the native LLVM includes and library
#
#  LLVM_INCLUDE_DIR - where to find llvm include files
#  LLVM_LIBRARY_DIR - where to find llvm libs
#  LLVM_CFLAGS      - llvm compiler flags
#  LLVM_LFLAGS      - llvm linker flags
#  LLVM_MODULE_LIBS - list of llvm libs for working with modules.
#  LLVM_FOUND       - True if llvm found.

if(CMAKE_CROSSCOMPILING)
    find_host_program(LLVM_CONFIG_EXECUTABLE NAMES llvm-config HINTS ~/usr/local/bin /usr/local/bin ${LLVM_PATH} ENV LLVM_PATH PATH_SUFFIXES bin DOC "llvm-config executable")
else()
    find_program(LLVM_CONFIG_EXECUTABLE NAMES llvm-config HINTS ~/usr/local/bin /usr/local/bin ${LLVM_PATH} ENV LLVM_PATH PATH_SUFFIXES bin DOC "llvm-config executable")
endif()

if (LLVM_CONFIG_EXECUTABLE)
    message(STATUS "Using llvm-config found at: ${LLVM_CONFIG_EXECUTABLE}")
    
    execute_process(
      COMMAND ${LLVM_CONFIG_EXECUTABLE} --includedir
      OUTPUT_VARIABLE LLVM_INCLUDE_DIR
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    execute_process(
      COMMAND ${LLVM_CONFIG_EXECUTABLE} --libdir
      OUTPUT_VARIABLE LLVM_LIBRARY_DIR
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    execute_process(
      COMMAND ${LLVM_CONFIG_EXECUTABLE} --bindir
      OUTPUT_VARIABLE LLVM_BINARY_DIR
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    
    execute_process(
      COMMAND ${LLVM_CONFIG_EXECUTABLE} --cppflags
      OUTPUT_VARIABLE LLVM_CFLAGS
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    execute_process(
      COMMAND ${LLVM_CONFIG_EXECUTABLE} --ldflags
      OUTPUT_VARIABLE LLVM_LFLAGS
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    execute_process(
      COMMAND ${LLVM_CONFIG_EXECUTABLE} --libs
      OUTPUT_VARIABLE LLVM_MODULE_LIBS
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )

	message( STATUS "LLVM libs: ${LLVM_MODULE_LIBS}")
else (LLVM_CONFIG_EXECUTABLE)
    
    if( DEFINED LLVM_PATH )
        message (STATUS "Using LLVM root location specified in LLVM_PATH variable: " ${LLVM_PATH})
        
        set(LLVM_INCLUDE_DIR ${LLVM_PATH}/include)
        set(LLVM_LIBRARY_DIR ${LLVM_PATH}/lib)
        string( CONFIGURE ${LLVM_LIBRARY_DIR} LLVM_LIBRARY_DIR)
        set(LLVM_BINARY_DIR ${LLVM_PATH}/bin)
        string( CONFIGURE ${LLVM_BINARY_DIR} LLVM_BINARY_DIR)
        set(LLVM_CFLAGS)
        set(LLVM_LFLAGS)
        set(LLVM_MODULE_LIBS
            LLVMX86Utils 
            LLVMTableGen 
            LLVMMICCodeGenerationEngine
            LLVMOpenCLAliasAnalysisSupport 
            LLVMMCJIT
            LLVMRuntimeDyld
            LLVMObject
            LLVMMCDisassembler
            LLVMLinker
            LLVMInterpreter
            LLVMIntelJITEvents
            LLVMInstrumentation
            LLVMJIT
            LLVMExecutionEngine
            LLVMDebugInfo
            LLVMBitWriter
            LLVMipo
            LLVMAsmParser
            LLVMArchive
            LLVMBitReader
            LLVMMICModuleJIT
)
           
if (NOT WIN32)            
        list(APPEND LLVM_MODULE_LIBS
             LLVMCppBackend
             LLVMCppBackendInfo
#            LLVMCBackend
#            LLVMCBackendInfo
        )
endif( NOT WIN32)
if( INCLUDE_MIC_DEVICE)
        list(APPEND LLVM_MODULE_LIBS
            LLVMY86AsmParser
            LLVMY86CodeGen
            LLVMY86Desc
            LLVMY86AsmPrinter
            LLVMY86Disassembler
            LLVMY86Info
            LLVMY86Utils
            )
endif( INCLUDE_MIC_DEVICE)
        list(APPEND LLVM_MODULE_LIBS
            LLVMX86Disassembler
            LLVMX86AsmParser
            LLVMX86CodeGen
            LLVMX86Desc
            LLVMSelectionDAG
            LLVMAsmPrinter
            LLVMMCParser
            LLVMCodeGen
            LLVMScalarOpts
            LLVMInstCombine
            LLVMTransformUtils
            LLVMipa
            LLVMAnalysis
            LLVMTarget
            LLVMX86AsmPrinter
            LLVMCore
            LLVMX86Info
            LLVMMC
            LLVMSupport        
            )
    elseif( EXISTS $ENV{LLVM_PATH} )
        message (STATUS "Using LLVM location specified in LLVM_PATH environment variable: " $ENV{LLVM_PATH} )
        set(LLVM_INCLUDE_DIR $ENV{LLVM_PATH}/include)
        set(LLVM_LIBRARY_DIR $ENV{LLVM_PATH}/lib)
        string( CONFIGURE ${LLVM_LIBRARY_DIR} LLVM_LIBRARY_DIR)
        set(LLVM_BINARY_DIR  $ENV{LLVM_PATH}/bin)
        string( CONFIGURE ${LLVM_BINARY_DIR} LLVM_BINARY_DIR)
        set(LLVM_CFLAGS)
        set(LLVM_LFLAGS)
        set(LLVM_MODULE_LIBS
            LLVMX86Utils 
            LLVMTableGen 
            LLVMMICCodeGenerationEngine
            LLVMOpenCLAliasAnalysisSupport 
            LLVMMCJIT
            LLVMRuntimeDyld
            LLVMObject
            LLVMMCDisassembler
            LLVMLinker
            LLVMInterpreter
            LLVMIntelJITEvents
            LLVMInstrumentation
            LLVMJIT
            LLVMExecutionEngine
            LLVMDebugInfo
            LLVMBitWriter
            LLVMipo
            LLVMAsmParser
            LLVMArchive
            LLVMBitReader
            LLVMMICModuleJIT
            )
if (NOT WIN32)
        list (APPEND LLVM_MODULE_LIBS
            LLVMCppBackend
            LLVMCppBackendInfo
#            LLVMCBackend
#            LLVMCBackendInfo
        )
endif( NOT WIN32)
if( INCLUDE_MIC_DEVICE)
        list( APPEND LLVM_MODULE_LIBS
            LLVMY86AsmParser
            LLVMY86CodeGen
            LLVMY86Desc
            LLVMY86AsmPrinter
            LLVMY86Disassembler
            LLVMY86Info
            LLVMY86Utils
            )
endif( INCLUDE_MIC_DEVICE)
        list(APPEND LLVM_MODULE_LIBS
            LLVMX86Disassembler
            LLVMX86AsmParser
            LLVMX86CodeGen
            LLVMX86Desc
            LLVMSelectionDAG
            LLVMAsmPrinter
            LLVMMCParser
            LLVMCodeGen
            LLVMScalarOpts
            LLVMInstCombine
            LLVMTransformUtils
            LLVMipa
            LLVMAnalysis
            LLVMTarget
            LLVMX86AsmPrinter
            LLVMCore
            LLVMX86Info
            LLVMMC
            LLVMSupport        
            )
    else()
        message( FATAL_ERROR "Can't find LLVM library. Please specify LLVM library location using either LLVM_PATH environment variable or defining LLVM_PATH parameter to CMAKE" )
    endif()
    
endif (LLVM_CONFIG_EXECUTABLE)

