set(TARGET_NAME oclopt)

include_directories(${CMAKE_SOURCE_DIR}/backend/passes)

link_directories(${LLVM_LIBRARY_DIR})

add_executable(${TARGET_NAME}
    ocl_opt.cpp
    ${CMAKE_SOURCE_DIR}/backend/passes/InitializePasses.h
    ${CMAKE_SOURCE_DIR}/backend/passes/OCLPassSupport.h
)

# Backend passes
set(OCL_BE_PASSES
    BuiltInFuncImport
    InstToFuncCall
    GenericAddressResolution
    PreventDivCrashes
    RelaxedPass
    ShiftZeroUpperBits
    BuiltinCallToInst
    Prefetch
    LinearIdResolver
    LLVMBarrier
    DebugInfoPass
    LLVMVectorizer
    Materialize
    Obfuscation
    ImplicitArgsAnalysis
    PrepareKernelArgs
    AddImplicitArgs
    OclFunctionAttrs
    BuiltinLibInfo
    ModuleCleanup
    BackendPassesWrappers
    LocalBuffers
    LocalBuffAnalysis
    ReduceAlignment
    DetectFunctionPtrCalls
    ResolveWICall
    CloneBlockInvokeFuncToKernel
    DetectRecursion
    ResolveBlockToStaticCall
    PatchCallbackArgs
    SmartGVN
    KernelInfo
)

# Dependencies of the passes
set(OCL_BE_PASSES_DEPS
    LoopUtils
    OclBackendUtils
    reflection_module
    name_mangle
    metadataapi
)

if (NOT INCLUDE_MIC_DEVICE)
if(NOT WIN32)
    # fix the case when LLVM is built with MIC support, but we want OCL without MIC support.
    if (${LLVM_COMPONENTS} MATCHES "(y86.*)")
        set(LLVM_MODULE_LIBS "-lLLVMY86Disassembler -lLLVMY86CodeGen -lLLVMMICModuleJIT -lLLVMY86AsmParser -lLLVMY86Desc -lLLVMY86Info -lLLVMY86AsmPrinter -lLLVMY86Utils ${LLVM_MODULE_LIBS} -lpcg -lsvml -lirc")
    endif()
else()
    # Try to find Y86 libraries on windows.
    # Release mode
    string(REPLACE "${CMAKE_CFG_INTDIR}" "Release" RELEASE_DIR ${LLVM_LIBRARY_DIR})
    find_library(IS_Y86_TARGET LLVMY86CodeGen ${RELEASE_DIR})
    if (IS_Y86_TARGET)
        target_link_libraries(${TARGET_NAME} optimized
            LLVMY86AsmParser
            LLVMY86CodeGen
            LLVMMICModuleJIT
            LLVMY86Desc
            LLVMY86AsmPrinter
            LLVMY86Disassembler
            LLVMY86Info
            LLVMY86Utils
            libpcg
            svml_dispmt
            libirc
            libmmt
            libdecimal
            )
    endif()
    # Debug mode
    string(REPLACE "${CMAKE_CFG_INTDIR}" "Debug" DEBUG_DIR ${LLVM_LIBRARY_DIR})
    find_library(IS_Y86_TARGET LLVMY86CodeGen ${DEBUG_DIR})
    if (IS_Y86_TARGET)
        target_link_libraries(${TARGET_NAME} debug
            LLVMY86AsmParser
            LLVMY86CodeGen
            LLVMMICModuleJIT
            LLVMY86Desc
            LLVMY86AsmPrinter
            LLVMY86Disassembler
            LLVMY86Info
            LLVMY86Utils
            libpcg
            svml_dispmt
            libirc
            libmmt
            libdecimal
            )
    endif()
endif()
endif (NOT INCLUDE_MIC_DEVICE)

target_link_libraries(${TARGET_NAME}
    ${OCL_BE_PASSES}
    ${OCL_BE_PASSES_DEPS}
    LLVMOptEngine
    ${LLVM_MODULE_LIBS}
)

if(NOT WIN32)
    if(NOT ANDROID)
        target_link_libraries(${TARGET_NAME} pthread)
    endif(NOT ANDROID)
    target_link_libraries(${TARGET_NAME} dl)
endif(NOT WIN32)

add_dependencies(${TARGET_NAME} metadataapi LoopUtils)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME})

if( CMAKE_CROSSCOMPILING )
    add_custom_target( oclopt_native ALL
      COMMAND make oclopt
      DEPENDS ConfigureNativeTargets
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/native
      COMMENT "Cross-Compile: Building Native ocl_opt tool"
    )
endif( CMAKE_CROSSCOMPILING )

install( TARGETS ${TARGET_NAME} RUNTIME DESTINATION bin)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER "Backend" )
