; RUN: SATest -BUILD -tsize=1 -pass-manager-type=lto -llvm-option=-debug-pass-manager -config=%S/pipeline_lto.tst.cfg 2>&1 | FileCheck %s
; RUN: SATest -BUILD -tsize=1 -llvm-option=-debug-pass-manager -config=%S/pipeline_lto.tst.cfg 2>&1 | FileCheck %s
; TODO:
;   check VPlan driver pass is not run when VPlan is enabled in buildPerModuleDefaultPipeline.
;   check CoerceWin64Types pass when SATest is enabled on Windows.

; CHECK:      Running pass: DPCPPPreprocessSPIRVFriendlyIRPass
; CHECK-NEXT: Running pass: SPIRV::SPIRVLowerConstExprPass
; CHECK-NEXT: Running pass: SPIRV::SPIRVToOCL20Pass
; CHECK-NEXT: Running pass: NameAnonGlobalPass
; CHECK-NEXT: Running pass: SpecializeConstantPass

#ifndef NDEBUG
; CHECK-NEXT: Running pass: VerifierPass
; CHECK-NEXT: Running analysis: VerifierAnalysis
#endif // #ifndef NDEBUG

; CHECK:      Running pass: DPCPPEqualizerPass
; CHECK-NEXT: Running analysis: BuiltinLibInfoAnalysis
; CHECK-NEXT: Running pass: SetPreferVectorWidthPass
; CHECK:      Running pass: InternalizeNonKernelFuncPass
; CHECK:      Running pass: FMASplitterPass
; CHECK:      Running pass: AddFunctionAttrsPass
; CHECK-NEXT: Running pass: SimplifyCFGPass
; CHECK:      Running pass: SROAPass
; CHECK:      Running pass: InstCombinePass
; CHECK:      Running pass: InstSimplifyPass
; CHECK:      Running pass: LinearIdResolverPass
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
; CHECK:      Running pass: ResolveVarTIDCallPass
; CHECK-NEXT: Running pass: InferArgumentAliasPass
; CHECK-NEXT: Running pass: DuplicateCalledKernels
; CHECK-NEXT: Running analysis: LocalBufferAnalysis
; CHECK-NEXT: Running pass: DPCPPKernelAnalysisPass
; CHECK:      Running pass: SimplifyCFGPass
; CHECK:      Running pass: WGLoopBoundariesPass
; CHECK:      Running pass: DCEPass
; CHECK:      Running pass: SimplifyCFGPass
; CHECK:      Running pass: DeduceMaxWGDimPass
; CHECK:      Running analysis: CallGraphAnalysis
; CHECK-NEXT: Running pass: InstToFuncCallPass

; CHECK-NOT:  Running pass: VecClonePass

; CHECK:      Running pass: ReqdSubGroupSizePass
; CHECK-NEXT: Running pass: SetVectorizationFactorPass
; CHECK:      Running analysis: VFAnalysis
; CHECK:      Running analysis: WeightedInstCountAnalysis
; CHECK:      Running pass: ResolveSubGroupWICallPass
; CHECK:      Running pass: OptimizeIDivAndIRemPass
; CHECK:      Running pass: PreventDivCrashesPass
; CHECK:      Running pass: InstCombinePass
; CHECK:      Running pass: GVNPass
; CHECK:      Running pass: VectorCombinePass
; CHECK:      Running pass: JumpThreadingPass

; CHECK:      Running pass: DPCPPKernelWGLoopCreatorPass
; CHECK:      Running pass: IndirectCallLowering
; CHECK:      Running pass: DCEPass
; CHECK:      Running pass: SimplifyCFGPass
; CHECK:      Running pass: RemoveRegionDirectivesPass
; CHECK:      Running pass: UnifyFunctionExitNodesPass

; CHECK:      Running pass: ReplaceScalarWithMaskPass
; CHECK:      Running pass: ResolveSubGroupWICallPass
; CHECK:      Running pass: DCEPass
; CHECK:      Running pass: SimplifyCFGPass
; CHECK:      Running pass: PromotePass

; CHECK:      Running pass: PhiCanonicalization
; CHECK:      Running pass: RedundantPhiNode
; CHECK:      Running pass: GroupBuiltinPass
; CHECK-NEXT: Running pass: BarrierInFunction
; CHECK:      Running pass: RemoveDuplicatedBarrierPass
; CHECK-NEXT: Running pass: SGBuiltinPass
; CHECK-NEXT: Running analysis: SGSizeAnalysisPass
; CHECK-NEXT: Running pass: SGBarrierPropagatePass
; CHECK-NEXT: Running pass: SGBarrierSimplifyPass
; CHECK-NEXT: Running pass: SGValueWidenPass
; CHECK-NEXT: Running pass: SGLoopConstructPass
; CHECK-NEXT: Running pass: VerifierPass
; CHECK-NEXT: Running analysis: VerifierAnalysis
; CHECK-NEXT: Running pass: ResolveSubGroupWICallPass
; CHECK-NEXT: Running pass: SplitBBonBarrier
; CHECK-NEXT: Running pass: ReduceCrossBarrierValues
; CHECK:      Running analysis: DataPerValueAnalysis
; CHECK:      Running analysis: DataPerBarrierAnalysis
; CHECK:      Running analysis: WIRelatedValueAnalysis
; CHECK:      Running analysis: DominanceFrontierAnalysis
; CHECK:      Running analysis: DominatorTreeAnalysis
; CHECK:      Running pass: KernelBarrier

; CHECK:      Running pass: PromotePass
; CHECK:      Running pass: LoopSimplifyPass
; CHECK:      Running pass: LICMPass
; CHECK:      Running pass: BuiltinLICMPass
; CHECK:      Running pass: LoopStridedCodeMotionPass
; CHECK:      Running analysis: LoopWIAnalysis

; CHECK:      Running pass: AddImplicitArgsPass
; CHECK:      Running analysis: CallGraphAnalysis
; CHECK:      Running analysis: LocalBufferAnalysis
; CHECK:      Running analysis: ImplicitArgsAnalysis
; CHECK:      Running pass: ResolveWICallPass
; CHECK-NEXT: Running analysis: CallGraphAnalysis
; CHECK:      Running pass: LocalBuffersPass
; CHECK-NEXT: Running analysis: LocalBufferAnalysis
; CHECK-NEXT: Running analysis: CallGraphAnalysis
; CHECK:      Running pass: GlobalOptPass
; CHECK:      Running pass: BuiltinImportPass
; CHECK:      Running pass: InternalizeGlobalVariablesPass
; CHECK:      Running pass: BuiltinCallToInstPass
; CHECK:      Running pass: ModuleInlinerWrapperPass
; CHECK:      Running pass: InlinerPass
; CHECK:      Running pass: PatchCallbackArgsPass
; CHECK:      Running pass: GlobalDCEPass
; CHECK:      Running pass: DeadArgumentEliminationPass
; CHECK:      Running pass: SROAPass
; CHECK:      Running pass: LoopSimplifyPass
; CHECK:      Running pass: LICMPass
; CHECK:      Running pass: LoopIdiomRecognizePass
; CHECK:      Running pass: LoopDeletionPass
; CHECK:      Running pass: LoopStridedCodeMotionPass
; CHECK:      Running pass: SimplifyCFGPass
; CHECK:      Running pass: PrepareKernelArgsPass
; CHECK-NEXT: Running analysis: ImplicitArgsAnalysis
; CHECK:      Running pass: SimplifyCFGPass
; CHECK:      Running pass: SROAPass
; CHECK:      Running pass: InstCombinePass
; CHECK:      Running pass: GVNPass
; CHECK:      Running pass: DSEPass
; CHECK:      Running pass: ADCEPass
; CHECK:      Running pass: EarlyCSEPass
; CHECK:      Running pass: InstCombinePass
; CHECK:      Running pass: CleanupWrappedKernelPass

; CHECK: Test program was successfully built.

