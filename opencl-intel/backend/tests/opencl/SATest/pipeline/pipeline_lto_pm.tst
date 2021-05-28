; RUN: SATest -BUILD -pass-manager-type=lto-new -debug-passes=Structure -config=%s.cfg 2>&1 | FileCheck %s

; CHECK:      Running pass: DPCPPEqualizerPass
; CHECK-NEXT: Running pass: LinearIdResolverPass
; CHECK-NEXT: Running analysis: CallGraphAnalysis
; CHECK-NEXT: Running pass: DPCPPKernelAnalysisPass

; CHECK:      Running pass: DPCPPKernelWGLoopCreatorPass

; CHECK:      Running pass: PhiCanonicalization
; CHECK-NEXT: Running analysis: DominatorTreeAnalysis
; CHECK-NEXT: Running analysis: PostDominatorTreeAnalysis
; CHECK-NEXT: Running pass: Intel Kernel RedundantPhiNode
; CHECK-NEXT: Running pass: Intel Kernel BarrierInFunction
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
; CHECK-NEXT: Running pass: BuiltinImportPass
; CHECK-NEXT: Running pass: PrepareKernelArgsPass
