; Testing that WholeProgramAnalysis is not invalidated until
; AndersensAA is called second time (just before LoopOpt) during LTO
; compilation pipeline. Currently, this test supports only legacy pass
; manager since AndersensAA is not called second time yet in new PM.
; llvm-lto tool is used instead of opt to simulate LTO pass pipeline.
; REQUIRES: asserts

; RUN: opt %s -o Andersens_WP_pm.bc
; RUN: llvm-lto -disable-verify -debug-pass=Executions -O3 Andersens_WP_pm.bc 2>&1 | FileCheck %s

; CHECK: Freeing Pass 'Intel IPO Prefetch' on Module

; CHECK: Executing Pass 'Whole program analysis' on Module
;
; CHECK-NOT: Freeing Pass 'Whole program analysis' on Module
;
; CHECK: Executing Pass 'Andersen Interprocedural AA' on Module
;
; CHECK-NOT: Freeing Pass 'Whole program analysis' on Module
;
; CHECK: Executing Pass 'Andersen Interprocedural AA' on Module
;
; CHECK: Freeing Pass 'Whole program analysis' on Module

define void @main()  {
  ret void
}
