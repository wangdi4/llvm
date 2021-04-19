; RUN: SATest -BUILD -pass-manager-type=lto-legacy -dump-time-passes=%t2 -config=%s.cfg
; RUN: FileCheck %s --input-file=%t2

; CHECK-DAG: AddImplicitArgsLegacy
; CHECK-DAG: DPCPPEqualizerLegacy
; CHECK-DAG: DPCPPKernelAnalysisLegacy
; CHECK-DAG: ImplicitArgsAnalysisLegacy
; CHECK-DAG: LocalBufferAnalysisLegacy
; CHECK-DAG: PrepareKernelArgsLegacy
; CHECK-DAG: ResolveWICallLegacy
; CHECK-DAG: WGLoopCreatorLegacy
; CHECK-DAG: PhiCanonicalization
; CHECK-DAG: Intel Kernel RedundantPhiNode
; CHECK-DAG: Intel Kernel DataPerValue Analysis
; CHECK-DAG: Intel Kernel DataPerBarrier Analysis
; CHECK-DAG: Intel Kernel WIRelatedValue Analysis
; CHECK-DAG: Intel Kernel BarrierInFunction
; CHECK-DAG: Intel Kernel SplitBBonBarrier
; CHECK-DAG: Intel Kernel Barrier
