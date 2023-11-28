; RUN: SATest -BUILD -pass-manager-type=lto -llvm-option=-debug-pass-manager -config=%S/pipeline_lto.tst.cfg 2>&1 | FileCheck %s
; RUN: SATest -BUILD -llvm-option=-debug-pass-manager -config=%S/pipeline_lto.tst.cfg 2>&1 | FileCheck %s
; RUN: SATest -BUILD -llvm-option=-debug-pass-manager=quiet -config=%S/pipeline_lto.tst.cfg 2>&1 | FileCheck %s -check-prefix=QUIET
; RUN: SATest -BUILD -llvm-option="-debug-pass-manager=verbose -verify-each-pass" -config=%S/pipeline_lto.tst.cfg 2>&1 | FileCheck %s -check-prefix=VERIFY-EACH

; Here we avoid adding too many CHECK-NEXT before LLVM standard passes, since
; it may be fragile if there is new analysis dependency change.

; TODO:
;   check CoerceWin64Types pass when SATest is enabled

; CHECK-NOT:  Running pass: SYCLPreprocessSPIRVFriendlyIRPass
; CHECK-NOT:  Running pass: SPIRV::SPIRVLowerConstExprPass
; CHECK:      Running pass: KernelTargetExtTypeLowerPass
; CHECK:      Running pass: SPIRV::SPIRVToOCL20Pass
; CHECK-NEXT: Running pass: NameAnonGlobalPass
; CHECK-NEXT: Running pass: SpecializeConstantPass

#ifndef NDEBUG
; CHECK-NEXT: Running pass: VerifierPass
; CHECK-NEXT: Running analysis: VerifierAnalysis
#endif // #ifndef NDEBUG

; CHECK:      Running pass: SYCLEqualizerPass
; CHECK-NEXT: Running analysis: BuiltinLibInfoAnalysis
; CHECK:      Running pass: ExternalizeGlobalVariablesPass
; CHECK-NEXT: Running pass: CoerceTypesPass
; CHECK-NEXT: Running pass: SetPreferVectorWidthPass
; CHECK:      Running pass: InternalizeNonKernelFuncPass
; CHECK:      Running pass: LinearIdResolverPass
; CHECK:      Running pass: AddFunctionAttrsPass
; CHECK:      Running pass: SimplifyCFGPass
; CHECK:      Running pass: SROAPass
; CHECK:      Running pass: InstCombinePass
; CHECK:      Running pass: InstSimplifyPass
; CHECK:      Running pass: ResolveVarTIDCallPass
; CHECK:      Running pass: SGRemapWICallPass
; CHECK:      Running pass: PromotePass
; CHECK:      Running pass: InferAddressSpacesPass
; CHECK-NEXT: Running pass: BuiltinCallToInstPass
; CHECK-NEXT: Running pass: IPSCCPPass

; CHECK:      Running pass: DetectRecursionPass
; CHECK:      Running pass: ReassociatePass
; CHECK:      Running pass: InferAddressSpacesPass
; CHECK:      Running pass: SimplifyCFGPass
; CHECK:      Running pass: SROAPass
; CHECK:      Running pass: EarlyCSEPass
; CHECK:      Running pass: PromotePass
; CHECK:      Running pass: InstCombinePass
; CHECK:      Running pass: SinCosFoldPass
; CHECK-NEXT: Running pass: MathLibraryFunctionsReplacementPass
; CHECK-NEXT: Running pass: UnifyFunctionExitNodesPass
; CHECK:      Running pass: SimplifyCFGPass
; CHECK:      Running pass: InstCombinePass
; CHECK:      Running pass: GVNHoistPass
; CHECK:      Running pass: DCEPass
; CHECK:      Running pass: InferArgumentAliasPass
; CHECK:      Running pass: DuplicateCalledKernelsPass
; CHECK-NEXT: Running analysis: CallGraphAnalysis
; CHECK-NEXT: Running analysis: LocalBufferAnalysis
; CHECK-NEXT: Running pass: SYCLKernelAnalysisPass
; CHECK:      Running pass: SimplifyCFGPass
; CHECK:      Running pass: WGLoopBoundariesPass
; CHECK:      Running pass: DCEPass
; CHECK:      Running pass: SimplifyCFGPass
; CHECK:      Running pass: LoopUnrollPass
; CHECK:      Running pass: DeduceMaxWGDimPass
; CHECK:      Running analysis: CallGraphAnalysis
; CHECK-NEXT: Running pass: InstToFuncCallPass
; CHECK-NEXT: Running pass: MathFuncSelectPass
; CHECK-NEXT: Running pass: ReqdSubGroupSizePass
; CHECK-NEXT: Running pass: SetVectorizationFactorPass
; CHECK:      Running analysis: VFAnalysis
; CHECK:      Running analysis: WeightedInstCountAnalysis
; CHECK:      Running pass: VectorVariantLowering
; CHECK-NEXT: Running pass: CreateSimdVariantPropagation
; CHECK-NEXT: Running pass: SGSizeCollectorPass
; CHECK-NEXT: Running pass: SGSizeCollectorIndirectPass
; CHECK-NEXT: Running pass: RequireAnalysisPass<llvm::VectorizationDimensionAnalysis, llvm::Module
; CHECK-NEXT: Running analysis: VectorizationDimensionAnalysis
; CHECK-NEXT: Running pass: SYCLKernelVecClonePass
; CHECK-NEXT: Invalidating analysis: InnerAnalysisManagerProxy<{{[llvm::]*}}FunctionAnalysisManager, {{[llvm::]*}}Module>
; CHECK-NEXT: Invalidating analysis: CallGraphAnalysis
; CHECK-NEXT: Invalidating analysis: VFAnalysis
; CHECK-NEXT: Invalidating analysis: VectorizationDimensionAnalysis
; CHECK-NEXT: Running pass: VectorVariantFillIn
; CHECK-NEXT: Running pass: UpdateCallAttrs

; CHECK:      Running pass: PromotePass
; CHECK:      Running pass: LoopSimplifyPass
; CHECK:      Running pass: LICMPass

; CHECK-NOT:  Running pass: VecClonePass
; CHECK:      Running pass: vpo::VPlanDriverLLVMPass

; CHECK:      Running pass: SYCLKernelPostVecPass
; CHECK:      Running pass: InstCombinePass
; CHECK:      Running pass: SimplifyCFGPass
; CHECK:      Running pass: PromotePass
; CHECK:      Running pass: ADCEPass
; CHECK:      Running pass: InstCombinePass
; CHECK:      Running pass: VectorKernelEliminationPass
; CHECK:      Running analysis: WeightedInstCountAnalysis
; CHECK:      Running pass: HandleVPlanMask
; CHECK-NEXT: Running pass: ResolveSubGroupWICallPass
; CHECK:      Running pass: OptimizeIDivAndIRemPass
; CHECK:      Running pass: PreventDivCrashesPass
; CHECK:      Running pass: InstCombinePass
; CHECK:      Running pass: GVNPass
; CHECK:      Running pass: VectorCombinePass
; CHECK:      Running pass: JumpThreadingPass

; CHECK:      Running pass: ImplicitGIDPass
; CHECK-NEXT: Running analysis: DataPerBarrierAnalysis
; CHECK-NEXT: Running pass: SYCLKernelWGLoopCreatorPass
; CHECK:      Running pass: IndirectCallLowering ;INTEL
; CHECK:      Running pass: DCEPass
; CHECK:      Running pass: SimplifyCFGPass
; CHECK:      Running pass: UnifyFunctionExitNodesPass

; CHECK:      Running pass: ReplaceScalarWithMaskPass
; CHECK:      Running pass: ResolveSubGroupWICallPass
; CHECK:      Running pass: DCEPass
; CHECK:      Running pass: SimplifyCFGPass
; CHECK:      Running pass: PromotePass

; CHECK:      Running pass: GroupBuiltinPass
; CHECK-NEXT: Running pass: BarrierInFunction
; CHECK:      Running pass: RemoveDuplicatedBarrierPass
; CHECK-NEXT: Running pass: SGBuiltinPass
; CHECK-NEXT: Running analysis: SGSizeAnalysisPass
; CHECK-NEXT: Running pass: SGBarrierPropagatePass
; CHECK-NEXT: Running pass: SGBarrierSimplifyPass
; CHECK-NEXT: Running pass: ImplicitGIDPass
; CHECK-NEXT: Running analysis: DataPerBarrierAnalysis
; CHECK-NEXT: Running pass: SGValueWidenPass
; CHECK:      Running pass: SGLoopConstructPass
; CHECK-NEXT: Running pass: VerifierPass
; CHECK-NEXT: Running analysis: VerifierAnalysis
; CHECK-NEXT: Running pass: ResolveSubGroupWICallPass
; CHECK-NEXT: Running pass: SplitBBonBarrier
; CHECK-NEXT: Running pass: ReduceCrossBarrierValues
; CHECK:      Running analysis: DataPerValueAnalysis
; CHECK:      Running analysis: WIRelatedValueAnalysis
; CHECK:      Running analysis: DominanceFrontierAnalysis
; CHECK:      Running analysis: DominatorTreeAnalysis
; CHECK:      Running pass: KernelBarrier

; CHECK:      Running pass: PromotePass
; CHECK:      Running pass: LoopSimplifyPass
; CHECK:      Running pass: BuiltinLICMPass
; CHECK:      Running pass: LICMPass
; CHECK:      Running pass: LoopStridedCodeMotionPass
; CHECK:      Running analysis: LoopWIAnalysis

; CHECK:      Running pass: AddImplicitArgsPass
; CHECK-NEXT: Running analysis: CallGraphAnalysis
; CHECK-NEXT: Running analysis: ImplicitArgsAnalysis
; CHECK:      Running pass: ResolveWICallPass
; CHECK-NEXT: Running analysis: CallGraphAnalysis
; CHECK:      Running pass: LocalBuffersPass
; CHECK-NEXT: Running analysis: LocalBufferAnalysis
; CHECK-NEXT: Running analysis: CallGraphAnalysis
; CHECK:      Running pass: BuiltinImportPass
; CHECK:      Running pass: InternalizeGlobalVariablesPass
; CHECK:      Running pass: BuiltinCallToInstPass
; CHECK:      Running pass: ModuleInlinerWrapperPass
; CHECK:      Running pass: InlinerPass
; CHECK:      Running pass: PatchCallbackArgsPass
; CHECK:      Running pass: DeadArgumentEliminationPass
; CHECK:      Running pass: ArgumentPromotionPass
; CHECK:      Running pass: SROAPass
; CHECK:      Running pass: LoopSimplifyPass
; CHECK:      Running pass: LICMPass
; CHECK:      Running pass: LoopIdiomRecognizePass
; CHECK:      Running pass: LoopDeletionPass
; CHECK:      Running pass: SimplifyCFGPass
; CHECK:      Running pass: PrepareKernelArgsPass
; CHECK-NEXT: Running analysis: ImplicitArgsAnalysis
; CHECK:      Running pass: GlobalOptPass
; CHECK:      Running pass: GlobalDCEPass
; CHECK:      Running pass: SROAPass on test
; CHECK:      Running pass: InstCombinePass on test
; CHECK:      Running pass: SimplifyCFGPass on test
; CHECK:      Running pass: GVNPass on test
; CHECK:      Running pass: DSEPass on test
; CHECK:      Running pass: ADCEPass on test
; CHECK:      Running pass: EarlyCSEPass on test
; CHECK:      Running pass: InstCombinePass on test
; CHECK:      Running pass: CleanupWrappedKernelPass

; CHECK: Test program was successfully built.


; need not to check all outputs
; QUIET:      Running pass: SPIRV::SPIRVToOCL20Pass
; QUIET-NEXT: Running pass: NameAnonGlobalPass
; QUIET-NEXT: Running pass: SpecializeConstantPass
; QUIET-NEXT: Running pass: VerifierPass
; QUIET-NEXT: Running pass: SYCLEqualizerPass
; QUIET-NEXT: Running pass: ExternalizeGlobalVariablesPass
; QUIET-NEXT: Running pass: CoerceTypesPass
; QUIET-NEXT: Running pass: SetPreferVectorWidthPass
; QUIET-NEXT: Running pass: InternalizeNonKernelFuncPass
; QUIET-NEXT: Running pass: LinearIdResolverPass
; QUIET-NEXT: Running pass: AddFunctionAttrsPass
; QUIET-NEXT: Running pass: SimplifyCFGPass
; QUIET-NEXT: Running pass: SROAPass
; QUIET-NEXT: Running pass: InstCombinePass
; QUIET-NEXT: Running pass: InstSimplifyPass
; QUIET:      Running pass: PromotePass
; QUIET-NEXT: Running pass: InferAddressSpacesPass
; QUIET-NEXT: Running pass: BuiltinCallToInstPass


; need not to check all outputs
; VERIFY-EACH:      Running pass: SPIRV::SPIRVToOCL20Pass
; VERIFY-EACH-NEXT: Verifying module main
; VERIFY-EACH-NEXT: Running pass: NameAnonGlobalPass
; VERIFY-EACH-NEXT: Verifying module main
; VERIFY-EACH-NEXT: Running pass: SpecializeConstantPass
; VERIFY-EACH-NEXT: Verifying module main
