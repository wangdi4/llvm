set(TARGET_NAME oclopt)

include_directories(${LLVM_INCLUDE_DIRS}
                    ${BACKEND_ROOT_DIR}/passes
                    ${BACKEND_ROOT_DIR}/utils
                    ${BACKEND_ROOT_DIR}/arch_headers)
link_directories(${LLVM_LIBRARY_DIRS})

add_executable(${TARGET_NAME}
    ocl_opt.cpp
    ${BACKEND_ROOT_DIR}/passes/InitializePasses.h
    ${BACKEND_ROOT_DIR}/passes/OCLPassSupport.h
)

# Backend passes
set(OCL_BE_PASSES
    AddImplicitArgs
    AutorunReplicator
    BackendPassesWrappers
    BuiltInFuncImport
    BuiltinCallToInst
    BuiltinLibInfo
    ChannelPipeTransformation
    CloneBlockInvokeFuncToKernel
    DebugInfoPass
    DetectRecursion
    GenericAddressResolution
    ImplicitArgsAnalysis
    InfiniteLoopCreator
    InstToFuncCall
    KernelInfo
    LLVMBarrier
    LLVMVectorizer
    SubGroupAdaptation
    LinearIdResolver
    LocalBuffAnalysis
    LocalBuffers
    Materialize
    ModuleCleanup
    Obfuscation
    OclFunctionAttrs
    PatchCallbackArgs
    PipeOrdering
    PipeSupport
    Prefetch
    PrepareKernelArgs
    PreventDivCrashes
    ReduceAlignment
    RelaxedPass
    ResolveBlockToStaticCall
    ResolveWICall
    ShiftZeroUpperBits
    SmartGVN
    SinCosFold
    PreLegalizeBools
    OCLAliasAnalysis
    PrintfArgumentsPromotion
    FMASplitter
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

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME})

install( TARGETS ${TARGET_NAME} RUNTIME DESTINATION tests/LITTests )
