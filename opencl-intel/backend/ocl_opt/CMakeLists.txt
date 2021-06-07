include_directories(${LLVM_INCLUDE_DIRS}
                    ${BACKEND_ROOT_DIR}/passes
                    ${BACKEND_ROOT_DIR}/optimizer
                    ${BACKEND_ROOT_DIR}/arch_headers)

# Backend passes
set(OCL_BE_PASSES
    AddFastMath
    AddImplicitArgs
    AddTLSGlobals
    AutorunReplicator
    BackendPassesWrappers
    BuiltinLibInfo
    ChannelPipeTransformation
    ChannelsUsageAnalysis
    CleanupWrappedKernels
    CoerceTypes
    CoerceWin64Types
    RemoveAtExit
    DebugInfoPass
    DetectRecursion
    ExternalizeGlobalVariables
    HandleVPlanMask
    ImplicitArgsAnalysis
    InfiniteLoopCreator
    InstToFuncCall
    InternalizeNonKernelFunc
    InternalizeGlobalVariables
    KernelInfo
    LLVMBarrier
    SGEmulation
    LLVMVectorizer
    SubGroupAdaptation
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
    SetPreferVectorWidth
    ShiftZeroUpperBits
    SinCosFold
    SmartGVN
    StripIntelIP
    OCLAliasAnalysis
    PrintfArgumentsPromotion
    SYCLPipesHack
    AddNTAttr
    ResolveVariableTIDCall
)

# Dependencies of the passes
set(OCL_BE_PASSES_DEPS
    LoopUtils
    OclBackendUtils
    reflection_module
)

# Remove unused LLVM libs for oclopt to avoid duplicate command line option
# registration issue in shared link.
set(UNUSED_LLVM_LIBS
  LLVMCoverage LLVMDebugInfoGSYM LLVMDebugInfoPDB LLVMDlltoolDriver
  LLVMDWARFLinker LLVMExecutionEngine LLVMFrontendOpenACC LLVMFuzzMutate
  LLVMIntelJITEvents LLVMInterpreter LLVMJITLink LLVMLibDriver LLVMLineEditor
  LLVMLTO LLVMMCA LLVMMCJIT LLVMMIRParser LLVMObjectYAML LLVMOption
  LLVMOrcError LLVMOrcJIT LLVMRuntimeDyld LLVMSymbolize LLVMTableGen
  LLVMWindowsManifest LLVMX86Disassembler LLVMXRay
)
list(REMOVE_ITEM LLVM_MODULE_LIBS ${UNUSED_LLVM_LIBS})

add_opencl_executable(${OCL_OPT}
  AnalysisWrappers.cpp
  BreakpointPrinter.cpp
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
  # Don't know why the following 3 components are not in LLVM_MODULE_LIBS. Add
  # them here as a workaround to resolve link issue.
  LLVMIntel_DTrans
  LLVMIntel_DTransOpt
  LLVMIntel_DTransAnalysis
)

set_target_properties (oclopt PROPERTIES FOLDER ${BACKEND_FOLDER_NAME})
