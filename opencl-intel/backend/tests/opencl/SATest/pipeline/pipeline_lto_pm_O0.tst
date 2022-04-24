; RUN: SATest -BUILD -pass-manager-type=lto-new -debug-passes=Structure -config=%S/pipeline_lto_O0.tst.cfg 2>&1 | FileCheck %s
; TODO:
;   check CoerceWin64Types pass when SATest is enabled on Windows.

; CHECK:      Running pass: DPCPPPreprocessSPIRVFriendlyIRPass
; CHECK-NEXT: Running pass: SPIRV::SPIRVLowerConstExprPass
; CHECK-NEXT: Running pass: SPIRV::SPIRVToOCL20Pass
; CHECK-NEXT: Running pass: NameAnonGlobalPass
#ifndef NDEBUG
; CHECK-NEXT: Running pass: VerifierPass
; CHECK-NEXT: Running analysis: VerifierAnalysis
#endif // #ifndef NDEBUG

; CHECK-NOT:  Running pass: InferAddressSpacesPass
; CHECK-NEXT: Running pass: DPCPPEqualizerPass
; CHECK-NEXT: Running analysis: BuiltinLibInfoAnalysis
; CHECK:      Running pass: DuplicateCalledKernels
; CHECK-NOT:  Running pass: InternalizeNonKernelFuncPass
; CHECK-NEXT: Running pass: AddFunctionAttrsPass
; CHECK-NEXT: Running pass: LinearIdResolverPass
; CHECK-NEXT: Running analysis: CallGraphAnalysis
; CHECK-NEXT: Running analysis: InnerAnalysisManagerProxy
; CHECK-NEXT: Running pass: BuiltinCallToInstPass

; CHECK:      Running pass: DPCPPKernelAnalysisPass

; CHECK-NOT:  Running pass: InferAddressSpacesPass

; CHECK:      Running pass: SetVectorizationFactorPass
; CHECK-NEXT: Running analysis: VFAnalysis
; CHECK-NEXT: Running pass: VectorVariantLowering
; CHECK-NEXT: Running pass: CreateSimdVariantPropagation
; CHECK-NEXT: Running pass: SGSizeCollectorPass
; CHECK-NEXT: Running pass: SGSizeCollectorIndirectPass
; CHECK-NEXT: Running pass: RequireAnalysisPass<llvm::VectorizationDimensionAnalysis, llvm::Module>
; CHECK-NEXT: Running analysis: VectorizationDimensionAnalysis
; CHECK-NEXT: Running pass: DPCPPKernelVecClonePass
; CHECK-NEXT: Invalidating analysis: VerifierAnalysis
; CHECK-NEXT: Invalidating analysis: CallGraphAnalysis
; CHECK-NEXT: Invalidating analysis: InnerAnalysisManagerProxy<llvm::FunctionAnalysisManager, llvm::Module>
; CHECK-NEXT: Invalidating analysis: VFAnalysis
; CHECK-NEXT: Invalidating analysis: VectorizationDimensionAnalysis
; CHECK-NEXT: Running pass: VectorVariantFillIn
; CHECK-NEXT: Running pass: UpdateCallAttrs

; CHECK-NOT:  Running pass: VecClonePass

; CHECK:      Running pass: DPCPPKernelPostVecPass
; CHECK:      Running pass: HandleVPlanMask

; CHECK:      Running pass: ResolveSubGroupWICallPass
; CHECK-NEXT: Running pass: DPCPPKernelWGLoopCreatorPass
; CHECK:      Running pass: IndirectCallLowering

; CHECK:      Running analysis: InnerAnalysisManagerProxy
; CHECK-NEXT: Running pass: PhiCanonicalization
; CHECK-NEXT: Running analysis: DominatorTreeAnalysis
; CHECK-NEXT: Running analysis: PostDominatorTreeAnalysis
; CHECK-NEXT: Running pass: Intel Kernel RedundantPhiNode
; CHECK-NEXT: Running pass: GroupBuiltin
; CHECK-NEXT: Running pass: Intel Kernel BarrierInFunction
; CHECK: Running pass: ResolveSubGroupWICallPass
; CHECK:      Running pass: Intel Kernel SplitBBonBarrier
; CHECK-NEXT: Running pass: Intel Kernel Barrier
; CHECK-NEXT: Running analysis: Intel Kernel DataPerBarrier Analysis
; CHECK-NEXT: Running analysis: Intel Kernel DataPerValue Analysis
; CHECK-NEXT: Running analysis: Intel Kernel WIRelatedValue Analysis

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
; CHECK-NEXT: Running analysis: InnerAnalysisManagerProxy
; CHECK-NEXT: Running pass: BuiltinCallToInstPass
; CHECK:      Running pass: AlwaysInlinerPass
; CHECK-NEXT: Running pass: PrepareKernelArgsPass
; CHECK:      Running pass: CleanupWrappedKernelPass
