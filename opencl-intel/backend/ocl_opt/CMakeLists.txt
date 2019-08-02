include_directories(${LLVM_INCLUDE_DIRS}
                    ${BACKEND_ROOT_DIR}/passes
                    ${BACKEND_ROOT_DIR}/optimizer
                    ${BACKEND_ROOT_DIR}/arch_headers)

# Backend passes
set(OCL_BE_PASSES
    AddImplicitArgs
    AddTLSGlobals
    AutorunReplicator
    BackendPassesWrappers
    BuiltInFuncImport
    BuiltinCallToInst
    BuiltinLibInfo
    ChannelPipeTransformation
    ChannelsUsageAnalysis
    CleanupWrappedKernels
    CoerceTypes
    DebugInfoPass
    DetectRecursion
    GenericAddressResolution
    ImplicitArgsAnalysis
    InfiniteLoopCreator
    InstToFuncCall
    InternalizeNonKernelFunc
    InternalizeGlobalVariables
    KernelInfo
    LLVMBarrier
    LLVMVectorizer
    SubGroupAdaptation
    LinearIdResolver
    LocalBuffAnalysis
    LocalBuffers
    LLVMEqualizer
    OclFunctionAttrs
    PatchCallbackArgs
    PipeIOTransformation
    PipeOrdering
    PipeSupport
    Prefetch
    PrepareKernelArgs
    PreventDivCrashes
    RelaxedPass
    ResolveBlockToStaticCall
    ResolveWICall
    ResolveSubGroupWICall
    ShiftZeroUpperBits
    SinCosFold
    SmartGVN
    StripIntelIP
    OCLAliasAnalysis
    PrintfArgumentsPromotion
    SYCLPipesHack
)

# Dependencies of the passes
set(OCL_BE_PASSES_DEPS
    LoopUtils
    OclBackendUtils
    metadataapi
    name_mangle
    reflection_module
)

add_opencl_executable(oclopt
  AnalysisWrappers.cpp
  BreakpointPrinter.cpp
  Debugify.cpp
  GraphPrinters.cpp
  NewPMDriver.cpp
  PassPrinters.cpp
  PrintSCC.cpp
  ocl_opt.cpp

  LINK_LIBS
  ${LLVM_MODULE_LIBS}
  ${OCL_BE_PASSES}
  ${OCL_BE_PASSES_DEPS}
  cl_sys_utils
)

set_target_properties (oclopt PROPERTIES FOLDER ${BACKEND_FOLDER_NAME})
