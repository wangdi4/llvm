; RUN: SATest -BUILD -pass-manager-type=lto-legacy -dump-time-passes=%t2 -config=%s.cfg
; RUN: FileCheck %s --input-file=%t2

; CHECK-DAG: PrepareKernelArgsLegacy
; CHECK-DAG: WGLoopCreatorLegacy
; CHECK-DAG: PhiCanonicalization
; CHECK-DAG: Intel Kernel RedundantPhiNode
; CHECK-DAG: Intel Kernel DataPerValue Analysis
; CHECK-DAG: Intel Kernel DataPerBarrier Analysis
; CHECK-DAG: Intel Kernel WIRelatedValue Analysis
; CHECK-DAG: Intel Kernel BarrierInFunction
; CHECK-DAG: Intel Kernel SplitBBonBarrier
; CHECK-DAG: Intel Kernel Barrier
; CHECK-DAG: AddImplicitArgsLegacy
; CHECK-DAG: ResolveWICallLegacy
; CHECK-DAG: DPCPPKernelAnalysisLegacy
; CHECK-DAG: DPCPPEqualizerLegacy
; CHECK-DAG: LocalBufferAnalysisLegacy
; CHECK-DAG: ImplicitArgsAnalysisLegacy
