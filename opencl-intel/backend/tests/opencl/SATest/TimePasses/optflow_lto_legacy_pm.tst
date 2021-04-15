; RUN: SATest -BUILD -pass-manager-type=lto-legacy -dump-time-passes=%t2 -config=%s.cfg
; RUN: FileCheck %s --input-file=%t2

; CHECK-DAG: PrepareKernelArgsLegacy
; CHECK-DAG: WGLoopCreatorLegacy
; CHECK-DAG: AddImplicitArgsLegacy
; CHECK-DAG: ResolveWICallLegacy
; CHECK-DAG: DPCPPKernelAnalysisLegacy
; CHECK-DAG: DPCPPEqualizerLegacy
; CHECK-DAG: LocalBufferAnalysisLegacy
; CHECK-DAG: ImplicitArgsAnalysisLegacy
