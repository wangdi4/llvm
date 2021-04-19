; RUN: SATest -BUILD -pass-manager-type=lto-new -dump-time-passes=%t2 -config=%s.cfg
; RUN: FileCheck %s --input-file=%t2

; CHECK-DAG: AddImplicitArgsPass
; CHECK-DAG: DPCPPEqualizerPass
; CHECK-DAG: DPCPPKernelAnalysisPass
; CHECK-DAG: DPCPPKernelWGLoopCreatorPass
; CHECK-DAG: PhiCanonicalization
; CHECK-DAG: Intel Kernel RedundantPhiNode
; CHECK-DAG: Intel Kernel DataPerValue Analysis
; CHECK-DAG: Intel Kernel DataPerBarrier Analysis
; CHECK-DAG: Intel Kernel WIRelatedValue Analysis
; CHECK-DAG: Intel Kernel BarrierInFunction
; CHECK-DAG: Intel Kernel SplitBBonBarrier
; CHECK-DAG: Intel Kernel Barrier
; CHECK-DAG: ImplicitArgsAnalysis
; CHECK-DAG: LocalBufferAnalysis
; CHECK-DAG: PrepareKernelArgsPass
; CHECK-DAG: ResolveWICallPass
