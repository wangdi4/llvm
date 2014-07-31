set(TARGET_NAME oclopt)

include_directories(${BACKEND_ROOT_DIR}/passes
                    ${BACKEND_ROOT_DIR}/utils
                    ${BACKEND_ROOT_DIR}/arch_headers)

link_directories(${LLVM_LIBRARY_DIR})

add_executable(${TARGET_NAME}
    ocl_opt.cpp
    ${BACKEND_ROOT_DIR}/passes/InitializePasses.h
    ${BACKEND_ROOT_DIR}/passes/OCLPassSupport.h
)

# Backend passes
set(OCL_BE_PASSES
    AddImplicitArgs
    BackendPassesWrappers
    BuiltInFuncImport
    BuiltinCallToInst
    BuiltinLibInfo
    CloneBlockInvokeFuncToKernel
    DebugInfoPass
    DetectFunctionPtrCalls
    DetectRecursion
    GenericAddressResolution
    ImplicitArgsAnalysis
    InstToFuncCall
    KernelInfo
    LLVMBarrier
    LLVMVectorizer
    LinearIdResolver
    LocalBuffAnalysis
    LocalBuffers
    Materialize
    ModuleCleanup
    Obfuscation
    OclFunctionAttrs
    PatchCallbackArgs
    Prefetch
    PrepareKernelArgs
    PreventDivCrashes
    ReduceAlignment
    RelaxedPass
    ResolveBlockToStaticCall
    ResolveWICall
    ShiftZeroUpperBits
    SmartGVN
)

# Dependencies of the passes
set(OCL_BE_PASSES_DEPS
    LoopUtils
    OclBackendUtils
    metadataapi
    name_mangle
    reflection_module
)


target_link_libraries(${TARGET_NAME}
    ${OCL_BE_PASSES}
    ${OCL_BE_PASSES_DEPS}
    LLVMOptEngine
    cl_sys_utils
    ${LLVM_MODULE_LIBS}
)

if(NOT WIN32)
    if(NOT ANDROID)
        target_link_libraries(${TARGET_NAME} pthread)
    endif(NOT ANDROID)
    target_link_libraries(${TARGET_NAME} dl)
endif(NOT WIN32)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME})

install( TARGETS ${TARGET_NAME} RUNTIME DESTINATION bin)

