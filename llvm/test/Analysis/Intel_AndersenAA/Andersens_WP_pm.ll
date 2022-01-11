; Testing that WholeProgramAnalysis is not invalidated until
; AndersensAA is called second time (just before LoopOpt) during LTO
; compilation pipeline.
; Old PM: llvm-lto tool is used instead of opt to simulate LTO pass pipeline.
; New PM: opt with 'lto<O3>' option is used to simulate LTO pass pipeline.
; REQUIRES: asserts

; RUN: opt %s -o Andersens_WP_pm.bc
; RUN: llvm-lto -use-new-pm=false -disable-verify -debug-pass=Executions -O3 Andersens_WP_pm.bc 2>&1 | FileCheck %s
; RUN: opt -passes='lto<O3>' -debug-pass-manager 2>&1 | FileCheck %s --check-prefix=CHECK-NEW-PM

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

; CHECK-NEW-PM: Running analysis: WholeProgramAnalysis on [module]
;
; CHECK-NEW-PM-NOT: Invalidating analysis: WholeProgramAnalysis on [module]
;
; CHECK-NEW-PM: Running analysis: AndersensAA on [module]
;
; CHECK-NEW-PM-NOT: Invalidating analysis: WholeProgramAnalysis on [module]
;
; CHECK-NEW-PM: Invalidating analysis: AndersensAA on [module]
;
; CHECK-NEW-PM: Running analysis: AndersensAA on [module]

define void @main()  {
  ret void
}
