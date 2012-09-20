set(TARGET_NAME x86_plugin)

set(TARGET_SOURCE_FILES  
    x86_plugin.cpp
)

set(TARGET_INCLUDE_FILES
    x86_archive.h
    x86_cvms.h
)

include_directories(
    ${BACKEND_ROOT_DIR}/optimizer
    ${BACKEND_ROOT_DIR}/utils
    ${BACKEND_ROOT_DIR}/arch_headers
    ${BACKEND_ROOT_DIR}/vectorizer
)

link_directories( "/Users/opencl/Documents/work/llvm/lib")
link_directories( "/System/Library/Frameworks/OpenCL.framework/Libraries")

add_library(${TARGET_NAME} SHARED 
    ${TARGET_SOURCE_FILES} 
    ${TARGET_INCLUDE_FILES}
)

set_target_properties(${TARGET_NAME} PROPERTIES
    LINK_FLAGS "-framework Foundation -framework CoreFoundation"
)

set (PASSES_LIBS
    # Backend passes
    AddImplicitArgs
    BuiltInFuncImport
    DebugInfoPass
    InstToFuncCall
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
    )

set (LLVM_LIBS
    # LLVM libs
    LLVMAnalysis
    LLVMVectorizer
    LLVMBarrier
    LLVMLinker
    LLVMX86AsmParser
    LLVMX86AsmPrinter
    LLVMX86CodeGen
    LLVMX86Disassembler
    LLVMX86Info
    LLVMTransformUtils
    LLVMCore
    LLVMTarget
    LLVMExecutionEngine
    LLVMJIT
    #LLVMMCJIT
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
    LLVMipa
    LLVMX86AsmPrinter
    LLVMMC
    LLVMAsmParser
    )

target_link_libraries(${TARGET_NAME}
    OclOptimizer
    name_mangle
    OclUtils
    cl2module
  
    # Backend passes
    ${PASSES_LIBS}
    
    # LLVM libs
    ${LLVM_LIBS}
    )

install( TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION bin)