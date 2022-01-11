; RUN: SATest -BUILD -pass-manager-type=lto-new -debug-passes=Structure -config=%s.cfg 2>&1 | FileCheck %s
; TODO:
;   check CoerceWin64Types pass when SATest is enabled on Windows.

; CHECK:      Running pass: DPCPPPreprocessSPIRVFriendlyIRPass
; CHECK-NEXT: Running pass: SPIRV::SPIRVToOCL20Pass
; CHECK-NEXT: Running pass: NameAnonGlobalPass
; CHECK-NEXT: Running pass: DPCPPEqualizerPass
; CHECK-NEXT: Running analysis: BuiltinLibInfoAnalysis
; CHECK:      Running pass: DuplicateCalledKernels
; CHECK-NOT:  Running pass: InternalizeNonKernelFuncPass
; CHECK-NEXT: Running pass: AddFunctionAttrsPass
; CHECK-NEXT: Running pass: LinearIdResolverPass
; CHECK-NEXT: Running analysis: CallGraphAnalysis
; CHECK-NEXT: Running analysis: InnerAnalysisManagerProxy
; CHECK-NEXT: Running pass: BuiltinCallToInstPass
; CHECK-NEXT: Running pass: DPCPPKernelAnalysisPass

; CHECK-NOT:  Running pass: VecClonePass

; CHECK:      Running pass: UnifyFunctionExitNodesPass
; CHECK:      Running pass: ResolveSubGroupWICallPass
; CHECK-NEXT: Running pass: DPCPPKernelWGLoopCreatorPass

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
