; RUN: SATest -BUILD -pass-manager-type=lto-new -dump-time-passes=%t2 -config=%s.cfg
; RUN: FileCheck %s --input-file=%t2

; CHECK-DAG: AddImplicitArgsPass
; CHECK-DAG: DPCPPEqualizerPass
; CHECK-DAG: DPCPPKernelAnalysisPass
; CHECK-DAG: DPCPPKernelWGLoopCreatorPass
; CHECK-DAG: ImplicitArgsAnalysis
; CHECK-DAG: LocalBufferAnalysis
; CHECK-DAG: PrepareKernelArgsPass
; CHECK-DAG: ResolveWICallPass
