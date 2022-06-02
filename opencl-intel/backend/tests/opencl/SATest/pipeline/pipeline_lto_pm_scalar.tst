; RUN: SATest -BUILD -tsize=1 -pass-manager-type=lto -llvm-option=-debug-pass-manager -config=%S/pipeline_lto.tst.cfg 2>&1 | FileCheck %s
; RUN: SATest -BUILD -tsize=1 -llvm-option=-debug-pass-manager -config=%S/pipeline_lto.tst.cfg 2>&1 | FileCheck %s
; TODO:
;   check VPlan driver pass is not run when VPlan is enabled in buildPerModuleDefaultPipeline.
;   check CoerceWin64Types pass when SATest is enabled on Windows.

; CHECK:      Running pass: DPCPPPreprocessSPIRVFriendlyIRPass
; CHECK-NEXT: Running pass: SPIRV::SPIRVLowerConstExprPass
; CHECK-NEXT: Running pass: SPIRV::SPIRVToOCL20Pass
; CHECK-NEXT: Running pass: NameAnonGlobalPass

#ifndef NDEBUG
; CHECK-NEXT: Running pass: VerifierPass
; CHECK-NEXT: Running analysis: VerifierAnalysis
#endif // #ifndef NDEBUG

; CHECK:      Running pass: InferAddressSpacesPass
; CHECK:      Running pass: DPCPPEqualizerPass
; CHECK-NEXT: Running analysis: BuiltinLibInfoAnalysis
; CHECK:      Running pass: DuplicateCalledKernels
; CHECK-NEXT: Running pass: InternalizeNonKernelFuncPass
#ifndef NDEBUG
; CHECK-NEXT: Invalidating analysis: VerifierAnalysis
#endif // #ifndef NDEBUG

; CHECK:      Running pass: AddFunctionAttrsPass
; CHECK-NEXT: Running pass: LinearIdResolverPass
; CHECK-NEXT: Running analysis: CallGraphAnalysis
; CHECK-NEXT: Running analysis: InnerAnalysisManagerProxy
; CHECK-NEXT: Running pass: BuiltinCallToInstPass

; CHECK:      Running pass: DPCPPKernelAnalysisPass
; CHECK-NEXT: Running analysis: LoopAnalysis
; CHECK-NEXT: Running pass: WGLoopBoundariesPass

; CHECK:      Running pass: ReassociatePass
; CHECK-NEXT: Running pass: InferAddressSpacesPass
; CHECK-NEXT: Running pass: SimplifyCFGPass
; CHECK-NEXT: Running pass: SROAPass
; CHECK:      Running pass: EarlyCSEPass
; CHECK-NEXT: Running pass: PromotePass
; CHECK-NEXT: Running pass: InstCombinePass
; CHECK:      Running pass: DeduceMaxWGDimPass
; CHECK-NEXT: Running pass: InstToFuncCallPass

; CHECK-NOT:  Running pass: DPCPPKernelVecClonePass
; CHECK-NOT:  Running pass: VecClonePass
; CHECK-NOT:  Running pass: vpo::VPlanDriverPass
; CHECK-NOT:  Running pass: DPCPPKernelPostVecPass
; CHECK-NOT:  Running pass: HandleVPlanMask

; CHECK:      Running pass: ResolveSubGroupWICallPass
; CHECK-NEXT: Running pass: DPCPPKernelWGLoopCreatorPass
; CHECK:      Running pass: IndirectCallLowering
; CHECK:      Running pass: ResolveSubGroupWICallPass

; CHECK:      Running pass: PhiCanonicalization
; CHECK-NEXT: Running analysis: DominatorTreeAnalysis
; CHECK-NEXT: Running analysis: PostDominatorTreeAnalysis
; CHECK:      Running pass: Intel Kernel RedundantPhiNode
; CHECK:      Running pass: GroupBuiltinPass
; CHECK-NEXT: Running pass: Intel Kernel BarrierInFunction
; CHECK:      Running pass: ResolveSubGroupWICallPass
; CHECK:      Running pass: Intel Kernel SplitBBonBarrier
; CHECK-NEXT: Running pass: ReduceCrossBarrierValues
; CHECK-NEXT: Running analysis: Intel Kernel DataPerValue Analysis
; CHECK-NEXT: Running analysis: Intel Kernel DataPerBarrier Analysis
; CHECK-NEXT: Running analysis: Intel Kernel WIRelatedValue
; CHECK-NEXT: Running analysis: InnerAnalysisManagerProxy<llvm::FunctionAnalysisManager, llvm::Module>
; CHECK-NEXT: Running analysis: DominanceFrontierAnalysis
; CHECK-NEXT: Running analysis: DominatorTreeAnalysis
; CHECK:      Running pass: Intel Kernel Barrier

; CHECK:      Running pass: AddImplicitArgsPass
; CHECK-NEXT: Running analysis: CallGraphAnalysis
; CHECK-NEXT: Running analysis: LocalBufferAnalysis
; CHECK-NEXT: Running analysis: ImplicitArgsAnalysis
; CHECK:      Running pass: ResolveWICallPass
; CHECK-NEXT: Running analysis: CallGraphAnalysis
; CHECK:      Running pass: LocalBuffersPass
; CHECK-NEXT: Running analysis: LocalBufferAnalysis
; CHECK-NEXT: Running analysis: CallGraphAnalysis
; CHECK:      Running pass: BuiltinImportPass
; CHECK:      Running analysis: InnerAnalysisManagerProxy
; CHECK-NEXT: Running pass: BuiltinCallToInstPass
; CHECK:      Running pass: InlinerPass
; CHECK:      Running pass: DeadArgumentEliminationPass
; CHECK:      Running pass: LoopSimplifyPass
; CHECK:      Running pass: LICMPass
; CHECK:      Running pass: LoopIdiomRecognizePass
; CHECK:      Running pass: LoopDeletionPass
; CHECK:      Running pass: PrepareKernelArgsPass
; CHECK:      Running pass: SimplifyCFGPass
; CHECK:      Running pass: SROAPass
; CHECK:      Running pass: InstCombinePass
; CHECK:      Running pass: GVNPass
; CHECK:      Running pass: DSEPass
; CHECK:      Running pass: ADCEPass
; CHECK:      Running pass: EarlyCSEPass
; CHECK:      Running pass: CleanupWrappedKernelPass
