; RUN: SATest -BUILD -lto-legacy-pm -dump-time-passes=%t2 -config=%s.cfg
; RUN: FileCheck %s --input-file=%t2

; CHECK-DAG: AddImplicitArgsLegacy
; CHECK-DAG: DPCPPEqualizerLegacy
; CHECK-DAG: DPCPPKernelAnalysisLegacy
; CHECK-DAG: ImplicitArgsAnalysisLegacy
; CHECK-DAG: LocalBufferAnalysisLegacy
; CHECK-DAG: PrepareKernelArgsLegacy
; CHECK-DAG: ResolveWICallLegacy
; CHECK-DAG: WGLoopCreatorLegacy
