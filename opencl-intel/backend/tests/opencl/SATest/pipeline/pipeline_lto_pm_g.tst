; RUN: SATest -BUILD -pass-manager-type=lto -llvm-option=-debug-pass-manager -config=%S/pipeline_lto_g.tst.cfg 2>&1 | FileCheck %s
; RUN: SATest -BUILD -llvm-option=-debug-pass-manager -config=%S/pipeline_lto_g.tst.cfg 2>&1 | FileCheck %s
; TODO:
;   check CoerceWin64Types pass when SATest is enabled on Windows.

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
; CHECK-NEXT: Running pass: InferArgumentAliasPass
; CHECK-NEXT: DuplicateCalledKernels
; CHECK-NEXT: Running analysis: LocalBufferAnalysis
; CHECK-NEXT: Running pass: SYCLKernelAnalysisPass
; CHECK:      Running pass: SimplifyCFGPass
; CHECK:      Running pass: WGLoopBoundariesPass
; CHECK:      Running pass: DCEPass
; CHECK:      Running pass: SimplifyCFGPass
; CHECK:      Running pass: LoopUnrollPass
; CHECK:      Running pass: DeduceMaxWGDimPass
; CHECK:      Running analysis: CallGraphAnalysis
; CHECK:      Running analysis: OptimizationRemarkEmitterAnalysis
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
; CHECK:      Running pass: SYCLKernelWGLoopCreatorPass
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
; CHECK-NEXT: Running analysis: InnerAnalysisManagerProxy<{{[llvm::]*}}FunctionAnalysisManager, {{[llvm::]*}}Module>
; CHECK-NEXT: Running pass: SGLoopConstructPass
; CHECK-NEXT: Running pass: VerifierPass
; CHECK-NEXT: Running analysis: VerifierAnalysis
; CHECK:      Running pass: ResolveSubGroupWICallPass
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

; CHECK:      Running pass: AddTLSGlobalsPass
; CHECK:      Running analysis: ImplicitArgsAnalysis
; CHECK-NEXT: Running analysis: LocalBufferAnalysis
; CHECK-NEXT: Running analysis: CallGraphAnalysis
; CHECK:      Running pass: ResolveWICallPass
; CHECK:      Running pass: LocalBuffersPass
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

